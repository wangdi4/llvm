/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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
#ifdef BUILD_FPGA_EMULATOR
        m_isChanged |= changePipeCall(CI, InstToRemove);
#endif
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
      .Case("__read_pipe_2", true)
      .Case("__read_pipe_4", true)
      .Case("__read_pipe_2_bl", true)
      .Case("__read_pipe_4_bl", true)
      .Case("__write_pipe_2", true)
      .Case("__write_pipe_4", true)
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

    // skip the first argument (the pipe), and the 2 last arguments (packet size
    // and max packets, they are not used by the BIs)
    assert(CI->getNumArgOperands() > 2 && "Unexpected number of arguments");
    for (size_t i = 1; i < CI->getNumArgOperands() - 2; ++i) {
      NewArgs.push_back(CI->getArgOperand(i));
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

static void FormOpenCLKernelsMetadata(Module &M) {
  assert(!M.getNamedMetadata("opencl.kernels") &&
    "Do not expect opencl.kernels Metadata");

  using namespace Intel::MetadataAPI;

  KernelList::KernelVectorTy kernels;

  for (auto &Func : M) {
    if ((Func.getCallingConv() == CallingConv::SPIR_KERNEL)
        && (!Func.isDeclaration())) {
      kernels.push_back(&Func);
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
