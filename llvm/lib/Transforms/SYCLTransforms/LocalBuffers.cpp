//=== LocalBuffers.cpp -  Map GlobalVariable __local to local buffer -========//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/LocalBuffers.h"

#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-local-buffers"

PreservedAnalyses LocalBuffersPass::run(Module &M, ModuleAnalysisManager &AM) {
  LocalBufferInfo *LBInfo = &AM.getResult<LocalBufferAnalysis>(M);
  if (!runImpl(M, LBInfo))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
bool LocalBuffersPass::runImpl(Module &M, LocalBufferInfo *LBInfo) {
  using namespace SYCLKernelMetadataAPI;

  this->M = &M;
  this->LBInfo = LBInfo;
  LBInfo->computeSize();
  const auto &LocalsMap = LBInfo->getDirectLocalsMap();

  HasTLSGlobals = CompilationUtils::hasTLSGlobals(M);
  Context = &M.getContext();

  DIFinder = DebugInfoFinder();
  DIFinder.processModule(M);

  // Get all kernels
  auto KernelsFunctionSet = CompilationUtils::getAllKernels(M);
  if (KernelsFunctionSet.empty())
    return false;

  // Run on all defined function in the module
  for (auto &Func : M) {
    Function *F = &Func;
    if (!F || F->isDeclaration()) {
      // Function is not defined inside module
      continue;
    }

    // pipes ctor is not a kernel
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(F))
      continue;

    if (!LocalsMap.lookup(F).empty())
      runOnFunction(F);
    if (KernelsFunctionSet.count(F)) {
      // We have a kernel, update metadata
      KernelInternalMetadataAPI(F).LocalBufferSize.set(
          LBInfo->getLocalsSize(F));
    }
  }

  updateDICompileUnitGlobals();

  // Most of local variable GVs are already handled. Erase them.
  for (GlobalVariable &GV : make_early_inc_range(M.globals())) {
    if (cast<PointerType>(GV.getType())->getAddressSpace() ==
        CompilationUtils::ADDRESS_SPACE_LOCAL) {
      GV.removeDeadConstantUsers();
      // May still have use in another global variable.
      [[maybe_unused]] auto HasOnlyConstantUser = [](GlobalVariable &GV) {
        for (User *U : GV.users()) {
          for (auto It = df_begin(U), E = df_end(U); It != E; ++It)
            if (!isa<Constant>(*It))
              return false;
        }
        return true;
      };
      assert((GV.use_empty() || HasOnlyConstantUser(GV)) &&
             "local variable isn't handled");
      if (GV.use_empty() && GV.isDiscardableIfUnused())
        GV.eraseFromParent();
    }
  }

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
        Replacement =
            CompilationUtils::createInstructionFromConstantWithReplacement(
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

void LocalBuffersPass::attachDebugInfoToLocalAddr(GlobalVariable *GV,
                                                  Value *LocalAddr) {
  SmallVector<DIGlobalVariableExpression *, 1> DIGVExprs;
  GV->getDebugInfo(DIGVExprs);

  DIBuilder DIB(*M, false);

  for (auto *DIGVExpr : DIGVExprs) {
    auto *DIGV = DIGVExpr->getVariable();
    if (DIGV->getScope() != SP)
      continue;
    auto *DIExpr = DIGVExpr->getExpression();
    DIExpr = DIExpression::prepend(DIExpr, DIExpression::DerefBefore);

    // Create a function-level Debug local variable
    auto *DILV = DIB.createAutoVariable(
        DIGV->getScope(), DIGV->getName(), DIGV->getFile(), DIGV->getLine(),
        DIGV->getType(), true, DINode::FlagArtificial);

    DIB.insertDbgValueIntrinsic(LocalAddr, DILV, DIExpr,
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
  const auto &LocalsMap = LBInfo->getDirectLocalsMap();

  // Iterate through local buffers directly used in F.
  for (GlobalVariable *GV : LocalsMap.lookup(F)) {
    // Retrieve the offset of the local buffer.
    size_t Offset = LBInfo->getLocalGVToOffset(GV);
    Type *Ty = CompilationUtils::getSLMBufferElementType(*Context);
    auto *Idx = ConstantInt::get(Type::getInt32Ty(*Context), Offset);
    auto *LocalAddr = Builder.CreateGEP(Ty, LocalMem, Idx, "");

    // Add bitcast to required/original pointer type.
    auto *Replacement = Builder.CreatePointerCast(LocalAddr, GV->getType());

    replaceAllUsesOfConstantWith(GV, dyn_cast<Instruction>(Replacement));

    if (SP)
      attachDebugInfoToLocalAddr(GV, LocalAddr);
  }
}

void LocalBuffersPass::runOnFunction(Function *F) {
  InsertPoint = &*inst_begin(F);
  SP = F->getSubprogram();

  // Getting the implicit arguments
  Value *LocalMem = 0;
  if (HasTLSGlobals) {
    LocalMem =
        CompilationUtils::getTLSGlobal(M, ImplicitArgsUtils::IA_SLM_BUFFER);
    assert(LocalMem && "TLS LocalMem is not found.");
    // Load the LocalMem pointer from TLS GlobalVariable
    IRBuilder<> Builder(InsertPoint);
    LocalMem =
        Builder.CreateLoad(cast<GlobalVariable>(LocalMem)->getValueType(),
                           LocalMem, "LocalMemBase");
  } else {
    CompilationUtils::getImplicitArgs(F, &LocalMem, nullptr, nullptr, nullptr,
                                      nullptr, nullptr, nullptr);
  }

  assert(LocalMem && "LocalMem should not be nullptr.");
  parseLocalBuffers(F, LocalMem);
}
