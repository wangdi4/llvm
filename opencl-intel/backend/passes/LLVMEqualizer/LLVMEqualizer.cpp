// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "LLVMEqualizer.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"
#include "OCLAddressSpace.h"
#include "MetadataAPI.h"
#include <NameMangleAPI.h>

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/GlobalStatus.h"

#include <algorithm>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

static cl::opt<bool> RemoveFPGAReg("remove-fpga-reg",
    cl::init(false), cl::Hidden,
    cl::desc("Remove __builtin_fpga_reg built-in calls."));

static cl::opt<bool> DemangleFPGAPipes("demangle-fpga-pipes",
    cl::init(false), cl::Hidden,
    cl::desc("Remove custom mangling from pipe built-ins"));

namespace intel {

// Basic block functors, to be applied on each block in the module.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  MaterializeBlockFunctor(BuiltinLibInfo &BLI,
                          llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove)
    : BLI(BLI), FuncDeclToRemove(FuncDeclToRemove) {}
  void operator()(llvm::BasicBlock &BB) {
    llvm::SmallVector<Instruction *, 4> InstToRemove;

    for (auto &b : BB) {
      if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&b)) {
        m_isChanged |= changeCallingConv(CI);

        if (RemoveFPGAReg) {
          m_isChanged |= removeFPGARegInst(CI, InstToRemove, FuncDeclToRemove);
        }

        if (DemangleFPGAPipes) {
          m_isChanged |=
            demangleFPGABICallInst(CI, InstToRemove, FuncDeclToRemove);
        }
      }
    }

    // Remove unused instructions
    for (auto inst : InstToRemove) {
      assert(inst->use_empty() && "Cannot erase used instructions");
      inst->eraseFromParent();
    }
  }

  bool changeCallingConv(llvm::CallInst *CI) {
    if ((llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
        (llvm::CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
      CI->setCallingConv(llvm::CallingConv::C);
      return true;
    }

    return false;
  }

  bool
  demangleFPGABICallInst(llvm::CallInst *CI,
                         llvm::SmallVectorImpl<Instruction *> &InstToRemove,
                         llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
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

    auto RTLs = BLI.getBuiltinModules();
    llvm::Module *PipesModule = nullptr;
    for (auto *M : RTLs) {
      if (M->getTypeByName("struct.__pipe_t")) {
        PipesModule = M;
        break;
      }
    }
    assert(PipesModule && "Module containing pipe built-ins not found!");

    assert(CI->getNumArgOperands() == 4 && "Unexpected number of arguments");
    SmallVector<Value *, 4> NewArgs;
    NewArgs.push_back(CI->getArgOperand(0)); // pipe argument

    if (FName.contains("_AS")) {
      FName = FName.drop_back(4);
      auto *Int8Ty = IntegerType::getInt8Ty(PipesModule->getContext());
      // We need to do a cast from global/local/private address spaces to
      // generic due to in backend we have pipe built-ins only with generic
      // address space.
      llvm::Type *I8PTy = llvm::PointerType::get(Int8Ty,
          Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Generic);
      auto ResArg = BitCastInst::CreatePointerBitCastOrAddrSpaceCast(
          CI->getArgOperand(1), I8PTy, "", CI);
      NewArgs.push_back(ResArg);
    } else {
      // copy packet argument as-is
      NewArgs.push_back(CI->getArgOperand(1));
    }

    // copy rest arguments
    for (size_t i = 2; i < CI->getNumArgOperands(); ++i)
      NewArgs.push_back(CI->getArgOperand(i));

    // Add _fpga suffix to pipe built-ins
    PipeKind Kind = CompilationUtils::getPipeKind(FName.str());
    Kind.FPGA = true;
    auto NewFName = CompilationUtils::getPipeName(Kind);

    llvm::Module *M = CI->getModule();
    llvm::Function *NewF = M->getFunction(NewFName);
    if (!NewF) {
      if (Kind.Blocking) {
        // Blocking built-ins are not declared in RTL, they are resolved
        // in PipeSupport instead.
        PipeKind NonBlockingKind = Kind;
        NonBlockingKind.Blocking = false;

        // Blocking built-ins differ from non blocking only by name, so we
        // import a non-blocking function to get a declaration ...
        NewF =
            CompilationUtils::importFunctionDecl(
                M, PipesModule->getFunction(
                    CompilationUtils::getPipeName(NonBlockingKind)));
        // ... and change it's name.
        NewF->setName(CompilationUtils::getPipeName(Kind));
      } else {
        NewF = CompilationUtils::importFunctionDecl(
                                  M, PipesModule->getFunction(NewFName));
      }
    }

    // With materilization of fpga pipe built-in calls we import new
    // declarations for them leaving old declarations unused. Add these unused
    // declarations with avoiding of duplications to the list of functions
    // to remove.
    FuncDeclToRemove.insert(F);

    llvm::CallInst *NewCI = llvm::CallInst::Create(NewF, NewArgs, "", CI);

    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setAttributes(CI->getAttributes());
    if (CI->isTailCall())
      NewCI->setTailCall();
    NewCI->setDebugLoc(CI->getDebugLoc());

    // Replace old call instruction with updated one.
    // SYCL blocking pipe built-ins unlike OpenCL has no return type, so instead
    // of replacing usages of the old instruction - just create a new one.
    InstToRemove.push_back(CI);
    if (CI->getType()->isVoidTy()) {
      assert(Kind.Blocking && "Only blocking pipes can have void return type!");
      return true;
    }
    CI->replaceAllUsesWith(NewCI);

    return true;
  }

  bool removeFPGARegInst(llvm::CallInst *CI,
                         llvm::SmallVectorImpl<Instruction *> &InstToRemove,
                         llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    if (!FName.startswith("llvm.fpga.reg."))
      return false;

    if (!FName.startswith("llvm.fpga.reg.struct."))
      CI->replaceAllUsesWith(CI->getArgOperand(0));
    else {
      Value *Dest = CI->getArgOperand(0);
      Value *Src = CI->getArgOperand(1);
      Dest->replaceAllUsesWith(Src);
    }
    FuncDeclToRemove.insert(F);
    InstToRemove.push_back(CI);
    return true;
  }

private:
  BuiltinLibInfo &BLI;
  llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  MaterializeFunctionFunctor(BuiltinLibInfo &BLI,
                             llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove)
    : BLI(BLI), FuncDeclToRemove(FuncDeclToRemove) {}

  void operator()(llvm::Function &F) {
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      m_isChanged = true;
    }
    MaterializeBlockFunctor bbMaterializer(BLI, FuncDeclToRemove);
    std::for_each(F.begin(), F.end(), bbMaterializer);
    m_isChanged |= bbMaterializer.isChanged();
  }

private:
  BuiltinLibInfo &BLI;
  llvm::SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

static void setBlockLiteralSizeMetadata(Function &F) {
  // Find all enqueue_kernel and kernel query calls
  for (const auto &EEF : *(F.getParent())) {
    if (!EEF.isDeclaration())
      continue;

    StringRef EEFName = EEF.getName();
    using namespace Intel::OpenCL::DeviceBackend;
    if (!(CompilationUtils::isEnqueueKernel(EEFName.str()) ||
          EEFName.equals("__get_kernel_work_group_size_impl") ||
          EEFName.equals("__get_kernel_preferred_work_group_size_multiple_impl")))
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
      auto KIMD = Intel::MetadataAPI::KernelInternalMetadataAPI(&F);
      KIMD.BlockLiteralSize.set(BlockSize);
      return;
    }
  }
}

static void FormOpenCLKernelsMetadata(Module &M) {
  assert(!M.getNamedMetadata("opencl.kernels") &&
    "Do not expect opencl.kernels Metadata");

  using namespace Intel::MetadataAPI;

  KernelList::KernelVectorTy kernels;

  for (auto &Func : M) {
    if (Func.isDeclaration())
      continue;
    if (Func.getCallingConv() != CallingConv::SPIR_KERNEL)
      continue;

    kernels.push_back(&Func);
    if (Func.getName().contains("_block_invoke_") &&
        Func.getName().endswith("_kernel")) {
      // Clang generates enqueued block invoke functions as kernels with
      // InternalLinkage, so ensure the linkage is External.
      // FIXME: It looks like a bug in clang
      Func.setLinkage(GlobalValue::ExternalLinkage);
      // Set BlockLiteralSizeMetadata for enqueued kernels
      setBlockLiteralSizeMetadata(Func);
    }
  }

  KernelList(&M).set(kernels);
}

// LLVMEqualizer

char LLVMEqualizer::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(LLVMEqualizer, "", "", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(LLVMEqualizer, "llvm-equalizer",
  "Prepares SPIR modules for BE consumption.",
  false, // Not CGF only pass.
  true
)

LLVMEqualizer::LLVMEqualizer() : ModulePass(ID) {}

bool LLVMEqualizer::runOnModule(llvm::Module &Module) {
  bool Ret = false;

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();

  llvm::SmallPtrSet<Function *, 4> FuncDeclToRemove;

  // form kernel list in the module.
  FormOpenCLKernelsMetadata(Module);
  MaterializeFunctionFunctor fMaterializer(BLI, FuncDeclToRemove);
  // Take care of calling conventions
  std::for_each(Module.begin(), Module.end(), fMaterializer);
  // Remove unused declarations
  for (auto *funcDecl : FuncDeclToRemove) {
    assert(funcDecl->use_empty() && "Cannot erase used declarations");
    funcDecl->eraseFromParent();
  }
  FuncDeclToRemove.clear();

  return Ret || fMaterializer.isChanged();
}

}

extern "C" {
llvm::ModulePass *createLLVMEqualizerPass() {
  return new intel::LLVMEqualizer();
}
}
