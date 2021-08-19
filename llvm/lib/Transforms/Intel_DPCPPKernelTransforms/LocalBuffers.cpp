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

namespace {

/// Legacy LocalBuffers Pass
class LocalBuffersLegacy : public ModulePass {
  LocalBuffersPass Impl;

public:
  static char ID;

  LocalBuffersLegacy(bool UseTLSGlobals = false)
      : ModulePass(ID), Impl(UseTLSGlobals || EnableTLSGlobals) {
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

  Context = &M.getContext();

  DIFinder = DebugInfoFinder();
  DIFinder.processModule(M);

  // Get all kernels
  auto kernelsFunctionSet = DPCPPKernelCompilationUtils::getAllKernels(M);

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
    if (kernelsFunctionSet.count(F)) {
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

Value *LocalBuffersPass::createInstructionFromConstantWithReplacement(
    Constant *C, Value *From, Value *To) {
  if (auto *CExpr = dyn_cast<ConstantExpr>(C)) {
    auto *Inst = CExpr->getAsInstruction();
    Inst->insertBefore(InsertPoint);
    Inst->replaceUsesOfWith(From, To);
    LLVM_DEBUG(dbgs() << "[createInstructionFromConstantWithReplacement]\n"
                      << "  Constant: " << *C << '\n'
                      << "  Instruction equivalent: " << *Inst << '\n');
    return Inst;
  }

  IRBuilder<> B(InsertPoint);
  if (SP)
    B.SetCurrentDebugLocation(DILocation::get(*Context, 0, 0, SP));

  Value *V = UndefValue::get(C->getType());
  for (unsigned i = 0; i < C->getNumOperands(); ++i) {
    Value *Op = C->getOperand(i);
    if (Op == From)
      Op = To;

    if (isa<ConstantVector>(C))
      V = B.CreateInsertElement(V, Op, i, "Insert");
    else if (isa<ConstantStruct>(C))
      V = B.CreateInsertValue(V, Op, i, "Insert");
    else {
      assert(false && "Unimplemented constant type conversion");
      return C;
    }
  }
  return V;
}

void LocalBuffersPass::replaceAllUsesOfConstantWith(Constant *From, Value *To) {
  assert(From && To && "shouldn't be nullptr");
  LLVM_DEBUG(dbgs() << "[replaceAllUsesOfConstantWith]\n"
                    << "  From: " << *From << '\n'
                    << "  To: " << *To << '\n');

  // Make an explicit copy of From's users, as we may erase user in the loop
  SmallVector<User *, 2> Users(From->users());
  for (auto *U : Users) {
    LLVM_DEBUG(dbgs() << "    Checking user: " << *U << '\n');
    if (auto *Inst = dyn_cast<Instruction>(U)) {
      if (Inst->getFunction() == InsertPoint->getFunction())
        Inst->replaceUsesOfWith(From, To);
    } else if (auto *CUser = dyn_cast<Constant>(U)) {
      if (CUser->use_empty())
        continue;

      Value *Inst =
          createInstructionFromConstantWithReplacement(CUser, From, To);
      replaceAllUsesOfConstantWith(CUser, Inst);
    }
  }
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
  GVToRemove.insert(GV);
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

  // Get all __local variables owned by F
  auto &LocalSet = LBInfo->getDirectLocals(F);

  unsigned int CurrLocalOffset = 0;

  // Iterate through local buffers
  for (auto *Local : LocalSet) {
    GlobalVariable *GV = cast<GlobalVariable>(Local);

    // Calculate required buffer size
    llvm::DataLayout DL(M);
    size_t ArraySize = DL.getTypeAllocSize(GV->getType()->getElementType());
    assert(0 != ArraySize && "zero array size!");
    // Now retrieve to the offset of the local buffer
    Type *Ty = LocalMem->getType()->getScalarType()->getPointerElementType();
    GetElementPtrInst *AddrOfLocal = GetElementPtrInst::Create(
        Ty, LocalMem,
        ConstantInt::get(IntegerType::get(*Context, 32), CurrLocalOffset), "",
        InsertPoint);

    // Now add bitcast to required/original pointer type
    CastInst *Replacement =
        CastInst::CreatePointerCast(AddrOfLocal, GV->getType(), "", InsertPoint);

    replaceAllUsesOfConstantWith(GV, Replacement);
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
    IRBuilder<> builder(InsertPoint);
    builder.CreateLoad(LocalMem->getType()->getPointerElementType(), LocalMem,
                       "LocalMemBase");
  } else {
    DPCPPKernelCompilationUtils::getImplicitArgs(F, &LocalMem, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr);
  }

  assert(LocalMem && "LocalMem should not be nullptr.");
  parseLocalBuffers(F, LocalMem);
}
