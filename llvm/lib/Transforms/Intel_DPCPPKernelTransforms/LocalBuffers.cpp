//=== LocalBuffers.cpp -  Map GlobalVariable __local to local buffer -========//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LocalBuffers.h"

#include "ImplicitArgsUtils.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-local-buffers"

extern bool EnableTLSGlobals;

namespace {

/// Legacy LocalBuffers Pass
class LocalBuffersLegacy : public ModulePass {
  LocalBuffersPass Impl;

public:
  static char ID;

  LocalBuffersLegacy(bool UseTLSGlobals = false)
      : ModulePass(ID), Impl(UseTLSGlobals) {
    initializeLocalBuffersLegacyPass(*PassRegistry::getPassRegistry());
  }

  ~LocalBuffersLegacy() {}

  StringRef getPassName() const override { return "LocalBuffersLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<LocalBufferAnalysisLegacy>();
    AU.setPreservesCFG();
  }
};

} // namespace

char LocalBuffersLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(LocalBuffersLegacy, DEBUG_TYPE,
                      "Map __local GlobalVariables to the local buffer", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(LocalBufferAnalysisLegacy)
INITIALIZE_PASS_END(LocalBuffersLegacy, DEBUG_TYPE,
                    "Map __local GlobalVariables to the local buffer", false,
                    false)

bool LocalBuffersLegacy::runOnModule(Module &M) {
  LocalBufferInfo *LBInfo =
      &getAnalysis<LocalBufferAnalysisLegacy>().getResult();
  return Impl.runImpl(M, LBInfo);
}

ModulePass *llvm::createLocalBuffersLegacyPass(bool UseTLSGlobals) {
  return new LocalBuffersLegacy(UseTLSGlobals);
}

PreservedAnalyses LocalBuffersPass::run(Module &M, ModuleAnalysisManager &AM) {
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);
  if (!runImpl(M, LBInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
bool LocalBuffersPass::runImpl(Module &M, LocalBufferInfo *LBInfo) {
  using namespace DPCPPKernelMetadataAPI;

  this->M = &M;
  this->LBInfo = LBInfo;

  UseTLSGlobals |= EnableTLSGlobals;
  Context = &M.getContext();

  DIFinder = DebugInfoFinder();
  DIFinder.processModule(M);

  // Get all kernels
  KernelsFunctionSet = DPCPPKernelCompilationUtils::getAllKernels(M);

  // Run on all defined function in the module
  for (auto &Func : M) {
    Function *F = &Func;
    if (!F || F->isDeclaration()) {
      // Function is not defined inside module
      continue;
    }

    // pipes ctor is not a kernel
    if (DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(F))
      continue;

    runOnFunction(F);
    if (KernelsFunctionSet.count(F)) {
      // We have a kernel, update metadata
      KernelInternalMetadataAPI(F).LocalBufferSize.set(
          LBInfo->getLocalsSize(F));
    }
  }

  updateDICompileUnitGlobals();

  // Safely erase useless GVs.
  for (auto *GV : GVToRemove)
    GV->eraseFromParent();

  return true;
}

/// Replaces all uses of Constant `From` with `To`. We can't simply
/// call `From->replaceAllUsesWith` because the users of `From` may
/// be ConstantExpr, ConstantAggregate. We have to convert those
/// Constant uses to instructions first.
static void replaceAllUsesOfConstantWith(Constant *From, Instruction *To) {
  assert(From && To && "shouldn't be nullptr");

  // Consider the following pattern, we need to find all user-chain paths from
  // the constant `From` to the the first instruction users.
  //
  // clang-format off
  //
  // store
  //   i64 sub (
  //     i64 ptrtoint (
  //       i8* getelementptr inbounds ([3 x i8], [3 x i8]* @CONSTANT, i64 0, i64 1) to i64
  //     ),
  //     i64 ptrtoint ([3 x i8]* @CONSTANT to i64)
  //   ),
  //   i64* %arrayidx
  //
  // The user-chain paths of @CONSTANT look like:
  //
  //                 @CONSTANT
  //                  /     \
  //                GEP   ptrtoint
  //                 |       |
  //             ptrtoint    |
  //                 \       /
  //                  \     /
  //                    sub
  //                     |
  //              store instruction
  //
  // clang-format on

  SmallVector<User *, 4> CurrentUserChain;
  // Helper function to build instructions from the current user chain.
  auto BuildInstructionsFromUserChain = [&](unsigned ChainLength,
                                            Instruction *InsertPoint,
                                            unsigned PHIReplaceIndex = 0) {
    assert(ChainLength > 0 && "User chain must have at least one user!");
    // Instruction node is always the last user on the chain.
    Instruction *LastInst =
        cast<Instruction>(CurrentUserChain[ChainLength - 1]);
    // It's a user chain outside this function, skip it.
    if (LastInst->getFunction() != To->getFunction())
      return;
    DenseMap<User *, Instruction *> UserToInstMap;
    UserToInstMap[From] = To;
    UserToInstMap[LastInst] = LastInst;
    LLVM_DEBUG(dbgs() << From->getName() << " user chain:\n");
    User *Prev = From;
    for (unsigned I = 0; I < ChainLength; ++I) {
      User *U = CurrentUserChain[I];
      LLVM_DEBUG(dbgs().indent(2) << *U << '\n');
      Instruction *Replacement = nullptr;
      if (UserToInstMap.count(U)) {
        Replacement = UserToInstMap[U];
        // If it's a PHI node, we cannot simply replaceUsesOfWith because PHI
        // node may have the same incoming value for different incoming BBs.
        // We can only replace the incoming value at PHIReplaceIndex.
        if (auto *PHI = dyn_cast<PHINode>(Replacement))
          PHI->setIncomingValue(PHIReplaceIndex, UserToInstMap[Prev]);
        else
          Replacement->replaceUsesOfWith(Prev, UserToInstMap[Prev]);
      } else {
        Replacement = DPCPPKernelCompilationUtils::
            createInstructionFromConstantWithReplacement(
                cast<Constant>(U), Prev, UserToInstMap[Prev], InsertPoint);
        UserToInstMap[U] = Replacement;
      }
      LLVM_DEBUG(dbgs().indent(4) << "--> " << *UserToInstMap[U] << '\n');
      Prev = U;
    }
  };

  // DFS starting from all From's users.
  // Stack element value stores the depth of the user on the user chain.
  MapVector<User *, unsigned> Stack;
  for (auto *U : From->users()) {
    assert((isa<Constant, Instruction>(U)) &&
           "Don't expect users other than Constant/Instruction");
    Stack.insert({U, 0});
  }
  while (!Stack.empty()) {
    User *Node = Stack.back().first;
    unsigned Depth = Stack.back().second;
    Stack.pop_back();
    // Expand/update the current user chain.
    assert(CurrentUserChain.size() >= Depth);
    if (CurrentUserChain.size() == Depth)
      CurrentUserChain.push_back(Node);
    else
      CurrentUserChain[Depth] = Node;
    // If we reaches a PHI instruction, we need to build instructions at the end
    // of the corresponding incoming basic block.
    if (auto *PHI = dyn_cast<PHINode>(Node)) {
      auto *PrevUser = Depth == 0 ? From : CurrentUserChain[Depth - 1];
      for (unsigned I = 0; I < PHI->getNumIncomingValues(); ++I) {
        // Find the incoming value(s) that need to be replaced.
        // Set insert point to the end of the incoming BB.
        if (PHI->getIncomingValue(I) == PrevUser) {
          auto *IncomingBB = PHI->getIncomingBlock(I);
          auto *IP = IncomingBB->getTerminator();
          BuildInstructionsFromUserChain(Depth + 1, IP, I);
        }
      }
      continue;
    }
    // If we reaches a non-PHI Instruction user node, convert the current user
    // chain to a sequence of instructions and insert them before Inst.
    if (auto *Inst = dyn_cast<Instruction>(Node)) {
      BuildInstructionsFromUserChain(Depth + 1, Inst);
      // No need to visit instruction node's users.
      continue;
    }
    for (auto *U : Node->users()) {
      assert((isa<Constant, Instruction>(U)) &&
             "Don't expect users other than Constant/Instruction");
      Stack.insert({U, Depth + 1});
    }
  }

  // Clean-up dead constant users after replacing them with instructions.
  From->removeDeadConstantUsers();
}

void LocalBuffersPass::attachDebugInfoToLocalMem(GlobalVariable *GV,
                                                 Value *LocalMem,
                                                 unsigned offset) {
  SmallVector<DIGlobalVariableExpression *, 1> DIGVExprs;
  GV->getDebugInfo(DIGVExprs);

  DIBuilder DIB(*M, false);

  for (auto *DIGVExpr : DIGVExprs) {
    auto *DIGV = DIGVExpr->getVariable();
    if (DIGV->getScope() != SP)
      continue;
    auto *DIExpr = DIGVExpr->getExpression();
    DIExpr = DIExpression::prepend(DIExpr, DIExpression::DerefBefore, offset);

    // Create a function-level Debug local variable
    auto *DILV = DIB.createAutoVariable(
        DIGV->getScope(), DIGV->getName(), DIGV->getFile(), DIGV->getLine(),
        DIGV->getType(), true, DINode::FlagArtificial);

    DIB.insertDbgValueIntrinsic(LocalMem, DILV, DIExpr,
                                DILocation::get(*Context, 0, 0, SP),
                                InsertPoint);
  }

  GVEToRemove.insert(DIGVExprs.begin(), DIGVExprs.end());
}

void LocalBuffersPass::updateDICompileUnitGlobals() {
  SmallVector<Metadata *> LiveGVs;
  for (auto *DICU : DIFinder.compile_units()) {
    LiveGVs.clear();

    for (auto *DIG : DICU->getGlobalVariables()) {
      if (!GVEToRemove.count(DIG))
        LiveGVs.push_back(DIG);
    }

    DICU->replaceGlobalVariables(MDTuple::get(*Context, LiveGVs));
  }
}

// Substitutes a pointer to local buffer, with argument passed within kernel
// parameters
void LocalBuffersPass::parseLocalBuffers(Function *F, Value *LocalMem) {

  IRBuilder<> Builder(InsertPoint);

  // Get all __local variables owned by F
  auto &LocalSet = LBInfo->getDirectLocals(F);

  bool IsKernel = KernelsFunctionSet.count(F);
  unsigned int CurrLocalOffset = 0;

  // Iterate through local buffers
  for (auto *Local : LocalSet) {
    GlobalVariable *GV = cast<GlobalVariable>(Local);

    // Calculate required buffer size
    llvm::DataLayout DL(M);
    size_t ArraySize = DL.getTypeAllocSize(GV->getValueType());
    assert(0 != ArraySize && "zero array size!");
    // Now retrieve to the offset of the local buffer
    Type *Ty = LocalMem->getType()->getScalarType()->getPointerElementType();
    auto *Idx = ConstantInt::get(IntegerType::get(*Context, 32),
                                 CurrLocalOffset);
    auto *pLocalAddr = Builder.CreateGEP(Ty, LocalMem, Idx, "");

    Value *Replacement = nullptr;
    if (IsKernel) {
      // For kernels, bitcast to required/original pointer type
      Replacement = Builder.CreatePointerCast(pLocalAddr, GV->getType());
    } else {
      // For non-kernel functions, load the pointer from LocalMemBase.
      auto *Cast = Builder.CreatePointerCast(pLocalAddr,
                                             GV->getType()->getPointerTo());
      Replacement = Builder.CreateLoad(GV->getType(), Cast);
    }

    replaceAllUsesOfConstantWith(GV, dyn_cast<Instruction>(Replacement));
    GVToRemove.insert(GV);
    if (SP)
      attachDebugInfoToLocalMem(GV, LocalMem, CurrLocalOffset);

    // Advance total implicit size
    CurrLocalOffset += ADJUST_SIZE_TO_MAXIMUM_ALIGN(ArraySize);
  }

  assert(CurrLocalOffset == LBInfo->getDirectLocalsSize(F) &&
         "CurrLocalOffset is not equal to local buffer size!");
}

void LocalBuffersPass::runOnFunction(Function *F) {
  InsertPoint = &*inst_begin(F);
  SP = F->getSubprogram();

  // Getting the implicit arguments
  Value *LocalMem = 0;
  if (UseTLSGlobals) {
    LocalMem = DPCPPKernelCompilationUtils::getTLSGlobal(
        M, ImplicitArgsUtils::IA_SLM_BUFFER);
    assert(LocalMem && "TLS LocalMem is not found.");
    // Load the LocalMem pointer from TLS GlobalVariable
    IRBuilder<> Builder(InsertPoint);
    LocalMem = Builder.CreateLoad(LocalMem->getType()->getPointerElementType(),
                                  LocalMem, "LocalMemBase");
  } else {
    DPCPPKernelCompilationUtils::getImplicitArgs(F, &LocalMem, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr);
  }

  assert(LocalMem && "LocalMem should not be nullptr.");
  parseLocalBuffers(F, LocalMem);
}
