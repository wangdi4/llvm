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

#include "Materialize.h"
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

// TODO: get better name to this pass.
// SPIR materialization now happens in front-end.
// This pass updates pipe built-in calls for FPGA emulator.

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

static cl::opt<bool> RemoveFPGAReg("remove-fpga-reg",
    cl::init(false), cl::Hidden,
    cl::desc("Remove __builtin_fpga_reg built-in calls."));

static cl::opt<bool> RemovePipeConstArgs("remove-pipe-const-args",
    cl::init(false), cl::Hidden,
    cl::desc("Remove packet size and alignment arguments from pipe built-ins"));

namespace intel {

// Basic block functors, to be applied on each block in the module.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  MaterializeBlockFunctor(BuiltinLibInfo &BLI) : BLI(BLI) {}
  void operator()(llvm::BasicBlock &BB) {
    llvm::SmallVector<Instruction *, 4> InstToRemove;

    for (llvm::BasicBlock::iterator b = BB.begin(), e = BB.end(); e != b; ++b) {
      if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&*b)) {
        m_isChanged |= changeCallingConv(CI);

        if (RemoveFPGAReg) {
          m_isChanged |= removeFPGARegInst(CI, InstToRemove);
        }

        if (RemovePipeConstArgs) {
          m_isChanged |= changePipeCall(CI, InstToRemove);
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

  bool changePipeCall(llvm::CallInst *CI,
                      llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();

    bool PipeBI = StringSwitch<bool>(FName)
      .Case("__read_pipe_2_AS0", true)
      .Case("__read_pipe_2_AS1", true)
      .Case("__read_pipe_2_AS3", true)
      .Case("__read_pipe_2", true)
      .Case("__read_pipe_4", true)
      .Case("__read_pipe_2_bl_AS0", true)
      .Case("__read_pipe_2_bl_AS1", true)
      .Case("__read_pipe_2_bl_AS3", true)
      .Case("__read_pipe_2_bl", true)
      .Case("__read_pipe_4_bl", true)
      .Case("__write_pipe_2", true)
      .Case("__write_pipe_2_AS0", true)
      .Case("__write_pipe_2_AS1", true)
      .Case("__write_pipe_2_AS2", true)
      .Case("__write_pipe_2_AS3", true)
      .Case("__write_pipe_4", true)
      .Case("__write_pipe_2_bl_AS0", true)
      .Case("__write_pipe_2_bl_AS1", true)
      .Case("__write_pipe_2_bl_AS2", true)
      .Case("__write_pipe_2_bl_AS3", true)
      .Case("__write_pipe_2_bl", true)
      .Case("__write_pipe_4_bl", true)
      .Case("__reserve_read_pipe", true)
      .Case("__reserve_write_pipe", true)
      .Default(false);

    if (!PipeBI)
      return false;

    auto RTLs = BLI.getBuiltinModules();

    llvm::StructType *InternalPipeTy = nullptr;
    llvm::Module *PipesModule = nullptr;
    for (auto *M : RTLs) {
      if ((InternalPipeTy = M->getTypeByName("struct.__pipe_t"))) {
        PipesModule = M;
        break;
      }
    }

    if (!InternalPipeTy) {
      llvm_unreachable("Cannot find __pipe_t struct in RTL.");
      return false;
    }

    auto *GlobalPipeTy = PointerType::get(InternalPipeTy,
          Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Global);

    auto PipeArg = BitCastInst::CreatePointerCast(CI->getArgOperand(0),
                                                  GlobalPipeTy, "", CI);
    SmallVector<Value *, 4> NewArgs;
    NewArgs.push_back(PipeArg);

    if (!FName.contains("_AS")) {
      // skip the first argument (the pipe), and the 2 last arguments (packet size
      // and max packets, they are not used by the BIs)
      assert(CI->getNumArgOperands() > 2 && "Unexpected number of arguments");
      for (size_t i = 1; i < CI->getNumArgOperands() - 2; ++i) {
        NewArgs.push_back(CI->getArgOperand(i));
      }
    } else {
      // For OpenCL 1.x we expect that built-in contains only 4 arguments.
      // Else branch handles OpenCL 1.x case.
      assert(CI->getNumArgOperands() == 4 && "Unexpected number of arguments");
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
    }

    llvm::SmallString<256> NewFName(FName);
    NewFName.append("_intel");

    llvm::Module *M = CI->getModule();

    llvm::Function *NewF = M->getFunction(NewFName);
    if (!NewF) {
      PipeKind Kind = CompilationUtils::getPipeKind(NewFName.str());
      if (Kind.Blocking) {
        // Blocking built-ins are not declared in RTL, they are resolved
        // in PipeSupport instead.
        PipeKind NonBlockingKind = Kind;
        NonBlockingKind.Blocking = false;

        // Blocking built-ins differ from non blocking only by name, so we
        // import a non-blocking function to get a declaration ...
        NewF = cast<Function>(
            CompilationUtils::importFunctionDecl(
                M, PipesModule->getFunction(
                    CompilationUtils::getPipeName(NonBlockingKind))));
        // ... and change it's name.
        NewF->setName(CompilationUtils::getPipeName(Kind));
      } else {
        NewF = cast<Function>(CompilationUtils::importFunctionDecl(
                                  M, PipesModule->getFunction(NewFName)));
      }
    }

    llvm::CallInst *NewCI = llvm::CallInst::Create(NewF, NewArgs, "", CI);

    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setAttributes(CI->getAttributes());
    if (CI->isTailCall())
      NewCI->setTailCall();
    NewCI->setDebugLoc(CI->getDebugLoc());

    // Replace old call instruction with updated one
    CI->replaceAllUsesWith(NewCI);
    InstToRemove.push_back(CI);

    return true;
  }

  bool removeFPGARegInst(llvm::CallInst *CI,
                         llvm::SmallVectorImpl<Instruction *> &InstToRemove) {
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
    InstToRemove.push_back(CI);
    return true;
  }

private:
  BuiltinLibInfo &BLI;
};

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  MaterializeFunctionFunctor(BuiltinLibInfo &BLI): BLI(BLI) {}

  void operator()(llvm::Function &F) {
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      m_isChanged = true;
    }
    MaterializeBlockFunctor bbMaterializer(BLI);
    std::for_each(F.begin(), F.end(), bbMaterializer);
    m_isChanged |= bbMaterializer.isChanged();
  }

private:
  BuiltinLibInfo &BLI;
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

// SpirMaterializer

char SpirMaterializer::ID = 0;

OCL_INITIALIZE_PASS_BEGIN(SpirMaterializer, "", "", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(SpirMaterializer, "spir-materializer",
  "Prepares SPIR modules for BE consumption.",
  false, // Not CGF only pass.
  true
)

SpirMaterializer::SpirMaterializer() : ModulePass(ID) {}

bool SpirMaterializer::runOnModule(llvm::Module &Module) {
  bool Ret = false;

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();

  // form kernel list in the module.
  FormOpenCLKernelsMetadata(Module);
  MaterializeFunctionFunctor fMaterializer(BLI);
  // Take care of calling conventions
  std::for_each(Module.begin(), Module.end(), fMaterializer);

  return Ret || fMaterializer.isChanged();
}

}

extern "C" {
llvm::ModulePass *createSpirMaterializer() {
  return new intel::SpirMaterializer();
}
}
