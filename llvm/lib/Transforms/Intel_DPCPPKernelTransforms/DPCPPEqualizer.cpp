//===- DPCPPEqualizerPass.cpp - DPC++ kernel equalizer --------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPEqualizer.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-equalizer"

namespace {

/// Base class for all functors, which supports immutability query.
class AbstractFunctor {
protected:
  bool IsChanged;

public:
  AbstractFunctor() : IsChanged(false) {}

  virtual ~AbstractFunctor() {}

  bool isChanged() const { return IsChanged; }
};

class FunctionFunctor : public AbstractFunctor {
public:
  virtual void operator()(Function &) = 0;
};

class BlockFunctor : public AbstractFunctor {
public:
  virtual void operator()(BasicBlock &) = 0;
};

// Basic block functors, to be applied on each block in the module.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  void operator()(BasicBlock &BB) override {
    for (auto &I : BB) {
      if (CallInst *CI = dyn_cast<CallInst>(&I)) {
        IsChanged |= changeCallingConv(CI);
      }
    }
  }

  bool changeCallingConv(CallInst *CI) {
    if ((CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
        (CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
      CI->setCallingConv(CallingConv::C);
      return true;
    }

    return false;
  }
};

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  void operator()(Function &F) override {
    CallingConv::ID CConv = F.getCallingConv();
    if (CallingConv::SPIR_FUNC == CConv || CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(CallingConv::C);
      IsChanged = true;
    }
    MaterializeBlockFunctor BBMaterializer;
    std::for_each(F.begin(), F.end(), BBMaterializer);
    IsChanged |= BBMaterializer.isChanged();
  }
};

/// Legacy DPCPPEqualizer pass.
class DPCPPEqualizerLegacy : public ModulePass {
  DPCPPEqualizerPass Impl;

public:
  static char ID;

  DPCPPEqualizerLegacy() : ModulePass(ID) {}

  ~DPCPPEqualizerLegacy() {}

  StringRef getPassName() const override { return "DPCPPEqualizerLegacy"; }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<CallGraphWrapperPass>();
    AU.setPreservesCFG();
  }
};

} // namespace

char DPCPPEqualizerLegacy::ID = 0;

INITIALIZE_PASS(DPCPPEqualizerLegacy, DEBUG_TYPE,
                "Setup kernel attribute and metadata", false, false)

ModulePass *llvm::createDPCPPEqualizerLegacyPass() {
  return new DPCPPEqualizerLegacy();
}

void DPCPPEqualizerPass::setBlockLiteralSizeMetadata(Function &F) {
  // Find all enqueue_kernel and kernel query calls.
  for (const auto &EEF : *(F.getParent())) {
    if (!EEF.isDeclaration())
      continue;

    StringRef EEFName = EEF.getName();
    if (!(DPCPPKernelCompilationUtils::isEnqueueKernel(EEFName.str()) ||
          EEFName.equals("__get_kernel_work_group_size_impl") ||
          EEFName.equals(
              "__get_kernel_preferred_work_group_size_multiple_impl")))
      continue;

    unsigned BlockInvokeIdx = (EEFName.startswith("__enqueue_kernel_"))
                                  ? (EEFName.contains("_events") ? 6 : 3)
                                  : 0;
    unsigned BlockLiteralIdx = BlockInvokeIdx + 1;

    for (auto *U : EEF.users()) {
      auto *EECall = dyn_cast<CallInst>(U);
      if (!EECall)
        continue;
      Value *BlockInvoke =
          EECall->getArgOperand(BlockInvokeIdx)->stripPointerCasts();
      if (BlockInvoke != &F)
        continue;
      Value *BlockLiteral =
          EECall->getArgOperand(BlockLiteralIdx)->stripPointerCasts();
      int64_t BlockSize = 0;
      if (auto *BlockAlloca = dyn_cast<AllocaInst>(BlockLiteral)) {
        BlockSize = F.getParent()->getDataLayout().getTypeAllocSize(
            BlockAlloca->getAllocatedType());
      } else if (auto *BlockGlobal = dyn_cast<Constant>(BlockLiteral)) {
        auto *BlockGlobalConst = cast<Constant>(BlockGlobal->getOperand(0));
        auto *Size = cast<ConstantInt>(BlockGlobalConst->getOperand(0));
        BlockSize = Size->getZExtValue();
      } else {
        llvm_unreachable("Unexpected instruction");
      }

      F.addFnAttr("block-literal-size", utostr(BlockSize));
      return;
    }
  }
}

void DPCPPEqualizerPass::formKernelsMetadata(Module &M) {
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getCallingConv() != CallingConv::SPIR_KERNEL)
      continue;

    if (F.getName().contains("_block_invoke_") &&
        F.getName().endswith("_kernel")) {
      // Clang generates enqueued block invoke functions as kernels with
      // InternalLinkage, so ensure the linkage is External.
      // FIXME: It looks like a bug in clang
      F.setLinkage(GlobalValue::ExternalLinkage);
      // Set block-literal-size attribute for enqueued kernels.
      setBlockLiteralSizeMetadata(F);
    }

    F.addFnAttr("sycl_kernel");
  }
}

bool DPCPPEqualizerPass::runImpl(Module &M) {
  // form kernel list in the module.
  formKernelsMetadata(M);

  MaterializeFunctionFunctor FuncMaterializer;
  // Take care of calling conventions
  std::for_each(M.begin(), M.end(), FuncMaterializer);

  return FuncMaterializer.isChanged();
}

PreservedAnalyses DPCPPEqualizerPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  (void)AM;
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
