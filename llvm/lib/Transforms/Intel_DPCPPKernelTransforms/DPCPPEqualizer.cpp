//===- DPCPPEqualizerPass.cpp - DPC++ kernel equalizer --------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPEqualizer.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace llvm::CompilationUtils;

#define DEBUG_TYPE "dpcpp-kernel-equalizer"

static cl::opt<bool>
    RemoveFPGAReg("dpcpp-remove-fpga-reg", cl::init(false), cl::Hidden,
                  cl::desc("Remove __builtin_fpga_reg built-in calls."));

static cl::opt<bool>
    DemangleFPGAPipes("dpcpp-demangle-fpga-pipes", cl::init(false), cl::Hidden,
                      cl::desc("Remove custom mangling from pipe built-ins"));

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
  MaterializeBlockFunctor(ArrayRef<Module *> BuiltinModules,
                          SmallPtrSetImpl<Function *> &FuncDeclToRemove)
      : BuiltinModules(BuiltinModules), FuncDeclToRemove(FuncDeclToRemove) {}

  void operator()(BasicBlock &BB) override {
    SmallVector<Instruction *, 4> InstToRemove;

    for (auto &I : BB) {
      if (CallInst *CI = dyn_cast<CallInst>(&I)) {
        IsChanged |= changeCallingConv(CI);

        if (RemoveFPGAReg)
          IsChanged |= removeFPGARegInst(CI, InstToRemove, FuncDeclToRemove);

        if (DemangleFPGAPipes)
          IsChanged |=
              demangleFPGAPipeBICall(CI, InstToRemove, FuncDeclToRemove);
      }
    }

    // Remove unused instructions.
    for (auto *I : InstToRemove)
      I->eraseFromParent();
  }

private:
  bool changeCallingConv(CallInst *CI) {
    if ((CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
        (CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
      CI->setCallingConv(CallingConv::C);
      return true;
    }

    return false;
  }

  bool demangleFPGAPipeBICall(CallInst *CI,
                              SmallVectorImpl<Instruction *> &InstToRemove,
                              SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    bool PipeBI = StringSwitch<bool>(FName)
                      .Case("__read_pipe_2", true)
                      .Case("__write_pipe_2", true)
                      .Case("__read_pipe_2_bl", true)
                      .Case("__write_pipe_2_bl", true)
                      .Case("__read_pipe_2_AS0", true)
                      .Case("__read_pipe_2_AS1", true)
                      .Case("__read_pipe_2_AS3", true)
                      .Case("__read_pipe_2_bl_AS0", true)
                      .Case("__read_pipe_2_bl_AS1", true)
                      .Case("__read_pipe_2_bl_AS3", true)
                      .Case("__write_pipe_2_AS0", true)
                      .Case("__write_pipe_2_AS1", true)
                      .Case("__write_pipe_2_AS2", true)
                      .Case("__write_pipe_2_AS3", true)
                      .Case("__write_pipe_2_bl_AS0", true)
                      .Case("__write_pipe_2_bl_AS1", true)
                      .Case("__write_pipe_2_bl_AS2", true)
                      .Case("__write_pipe_2_bl_AS3", true)
                      .Default(false);

    if (!PipeBI)
      return false;

    Module *PipesModule = nullptr;
    for (auto *M : BuiltinModules) {
      if (StructType::getTypeByName(M->getContext(), "struct.__pipe_t")) {
        PipesModule = M;
        break;
      }
    }
    assert(PipesModule && "Module containing pipe built-ins not found");

    assert(CI->arg_size() == 4 && "Unexpected number of arguments");
    SmallVector<Value *, 4> NewArgs;
    NewArgs.push_back(CI->getArgOperand(0));

    IRBuilder<> Builder(CI);

    if (FName.contains("_AS")) {
      FName = FName.drop_back(4);
      auto *Int8Ty = IntegerType::getInt8Ty(PipesModule->getContext());
      // We need to do a cast from global/local/private address spaces to
      // generic due to in backend we have pipe built-ins only with generic
      // address space.
      auto *I8PTy = PointerType::get(Int8Ty, ADDRESS_SPACE_GENERIC);
      auto *ResArg = Builder.CreatePointerBitCastOrAddrSpaceCast(
          CI->getArgOperand(1), I8PTy);
      NewArgs.push_back(ResArg);
    } else {
      // Copy packet argument as-is.
      NewArgs.push_back(CI->getArgOperand(1));
    }

    // Copy rest arguments.
    for (size_t I = 2; I < CI->arg_size(); ++I)
      NewArgs.push_back(CI->getArgOperand(I));

    // Add _fpga suffix to pipe built-ins.
    PipeKind Kind = getPipeKind(FName.str());
    Kind.FPGA = true;
    auto NewFName = getPipeName(Kind);

    Module *M = CI->getModule();
    Function *NewF = M->getFunction(NewFName);
    if (!NewF) {
      if (Kind.Blocking) {
        // Blocking built-ins are not declared in RTL, they are resolved in
        // PipeSupport instead.
        PipeKind NonBlockingKind = Kind;
        NonBlockingKind.Blocking = false;

        // Blocking built-ins differ from non-blocking only by name, so we
        // import a non-blocking function to get a declaration ...
        NewF = importFunctionDecl(
            M, PipesModule->getFunction(getPipeName(NonBlockingKind)),
            /*DuplicateIfExists*/ true);
        NewF->setName(getPipeName(Kind));
      } else {
        NewF = importFunctionDecl(M, PipesModule->getFunction(NewFName));
      }
    }

    // With materialization of fpga pipe built-in calls, we import new
    // declarations for them, leaving old declarations unused. Add these unused
    // declarations with avoiding of duplications to the list of functions to
    // remove.
    FuncDeclToRemove.insert(F);

    auto *NewCI = Builder.CreateCall(NewF, NewArgs);
    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setAttributes(CI->getAttributes());
    if (CI->isTailCall())
      NewCI->setTailCall();

    // Replace old call instruction with updated one.
    InstToRemove.push_back(CI);
    if (CI->getType()->isVoidTy()) {
      // SYCL blocking pipe built-ins unlike OpenCL have no return type, so
      // instead of replacing uses of the old instruction - just create a new
      // one.
      assert(Kind.Blocking && "Only blocking pipes can have void return type!");
      return true;
    }
    CI->replaceAllUsesWith(NewCI);

    return true;
  }

  bool removeFPGARegInst(CallInst *CI,
                         SmallVectorImpl<Instruction *> &InstToRemove,
                         SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    if (!FName.startswith("llvm.fpga.reg"))
      return false;

    if (!FName.startswith("llvm.fpga.reg.struct."))
      CI->replaceAllUsesWith(CI->getArgOperand(0));
    else {
      Value *Dst = CI->getArgOperand(0);
      Value *Src = CI->getArgOperand(1);
      Dst->replaceAllUsesWith(Src);
    }

    FuncDeclToRemove.insert(F);
    InstToRemove.push_back(CI);
    return true;
  }

private:
  ArrayRef<Module *> BuiltinModules;
  SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  MaterializeFunctionFunctor(ArrayRef<Module *> BuiltinModules,
                             SmallPtrSetImpl<Function *> &FuncDeclToRemove)
      : BuiltinModules(BuiltinModules), FuncDeclToRemove(FuncDeclToRemove) {}

  void operator()(Function &F) override {
    CallingConv::ID CConv = F.getCallingConv();
    if (CallingConv::SPIR_FUNC == CConv || CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(CallingConv::C);
      IsChanged = true;
    }
    MaterializeBlockFunctor BBMaterializer(BuiltinModules, FuncDeclToRemove);
    std::for_each(F.begin(), F.end(), BBMaterializer);
    IsChanged |= BBMaterializer.isChanged();
  }

private:
  ArrayRef<Module *> BuiltinModules;
  SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

} // namespace
void DPCPPEqualizerPass::setBlockLiteralSizeMetadata(Function &F) {
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(&F);
  // Find all enqueue_kernel and kernel query calls.
  for (const auto &EEF : *(F.getParent())) {
    if (!EEF.isDeclaration())
      continue;

    StringRef EEFName = EEF.getName();
    if (!(isEnqueueKernel(EEFName.str()) ||
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

      KIMD.BlockLiteralSize.set(BlockSize);
      return;
    }
  }
}

void DPCPPEqualizerPass::formKernelsMetadata(Module &M) {
  assert(!M.getNamedMetadata("sycl.kernels") &&
         "Do not expect sycl.kernels Metadata");

  using namespace DPCPPKernelMetadataAPI;

  KernelList::KernelVectorTy Kernels;

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getCallingConv() != CallingConv::SPIR_KERNEL)
      continue;

    Kernels.push_back(&F);

    // OpenCL/SYCL/SPIR-V kernel could have internal linkage since spec doesn't
    // mandate kernel to have external linkage.
    F.setLinkage(GlobalValue::ExternalLinkage);

    if (F.getName().contains("_block_invoke_") &&
        F.getName().endswith("_kernel")) {
      // Set block-literal-size attribute for enqueued kernels.
      setBlockLiteralSizeMetadata(F);
    }
  }
  DPCPPKernelMetadataAPI::KernelList KernelList(M);
  KernelList.set(Kernels);
}

bool DPCPPEqualizerPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  BuiltinModules = BLI->getBuiltinModules();

  // form kernel list in the module.
  formKernelsMetadata(M);

  SmallPtrSet<Function *, 4> FuncDeclToRemove;
  MaterializeFunctionFunctor FuncMaterializer(BuiltinModules, FuncDeclToRemove);

  // Take care of calling conventions.
  std::for_each(M.begin(), M.end(), FuncMaterializer);

  // Remove unused declarations.
  for (auto *FDecl : FuncDeclToRemove)
    FDecl->eraseFromParent();

  return FuncMaterializer.isChanged();
}

PreservedAnalyses DPCPPEqualizerPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  BuiltinLibInfo *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  if (!runImpl(M, BLI))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
