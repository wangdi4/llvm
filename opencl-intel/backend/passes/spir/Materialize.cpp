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
// TODO: move data layout materialization to separate pass or optimizer.

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";          // Used for RH64/SLES64.
const char *PC_LIN32 = "i686-pc-linux";            // Used for Android.
const char *PC_WIN32 = "i686-pc-win32-msvc-elf";   // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32-msvc-elf"; // Win 64 bit.

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
      .Case("__write_pipe_2", true)
      .Case("__write_pipe_4", true)
      .Case("__reserve_read_pipe", true)
      .Case("__reserve_write_pipe", true)
      .Default(false);

    if (!PipeBI)
      return false;

    auto m_runtimeModuleList = BLI.getBuiltinModules();

    llvm::StructType *InternalPipeTy = nullptr;
    llvm::Module *PipesModule = nullptr;
    for (auto *M : m_runtimeModuleList) {
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

    llvm::Function *NewF = cast<Function>(
        CompilationUtils::importFunctionDecl(CI->getParent()->getModule(),
                                             PipesModule->getFunction(
                                                 NewFName)));

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

static void WarpFunctionMetadata(Module &M) {
  if (M.getNamedMetadata("opencl.kernels") != NULL)
    return;

  auto OpenCLKernels = M.getOrInsertNamedMetadata("opencl.kernels");
  for (auto &Func : M) {
    if (Func.getCallingConv() == CallingConv::SPIR_KERNEL) {

      llvm::SmallVector<StringRef, 7> Names = {
          "kernel_arg_addr_space", "kernel_arg_access_qual",
          "kernel_arg_type",       "kernel_arg_base_type",
          "kernel_arg_type_qual",  "kernel_arg_name",
          "vec_type_hint",         "work_group_size_hint",
          "reqd_work_group_size"};

      llvm::SmallVector<Metadata *, 7> Args;
      Args.push_back(ConstantAsMetadata::get(&Func));

      for (const auto &I : Names) {
        auto First = MDString::get(M.getContext(), I);
        auto Second = Func.getMetadata(I);
        if (!Second)
          continue; // No such metadata.

        llvm::SmallVector<Metadata *, 2> Mdvector;
        Mdvector.push_back(First);
        for (auto &O : Second->operands()) {
          Mdvector.push_back(O);
        }
        Args.push_back(MDTuple::get(M.getContext(), Mdvector));
      }

      OpenCLKernels->addOperand(MDTuple::get(M.getContext(), Args));
    }
  }
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
  WarpFunctionMetadata(Module);

  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();

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

void UpdateTargetTriple(llvm::Module *pModule) {
  std::string triple = pModule->getTargetTriple();

  // Force ELF codegen on Windows (MCJIT does not support COFF format)
  if (((triple.find("win32") != std::string::npos) ||
       (triple.find("windows") != std::string::npos)) &&
      triple.find("-elf") == std::string::npos) {
    pModule->setTargetTriple(triple + "-elf"); // transforms:
                                               // x86_64-pc-win32
                                               // i686-pc-win32
                                               // to:
                                               // x86_64-pc-win32-elf
                                               // i686-pc-win32-elf
  }
}

void materializeSpirDataLayout(llvm::Module &M) {
  llvm::StringRef Triple(M.getTargetTriple());
  if (!Triple.startswith("spir"))
    return;

  // Replace the triple with that of the actual host, in case the triple is
  // SPIR.
  // Statically assigning the triple by the host's identity.
  Triple =
#if defined(_M_X64)
      intel::PC_WIN64;
#elif defined(__LP64__)
      intel::PC_LIN64;
#elif defined(_WIN32)
      intel::PC_WIN32;
#elif defined(__ANDROID__)
      intel::PC_LIN32;
#else
#error "Unsupported host platform"
#endif
  M.setTargetTriple(Triple);

  // Since we codegen ELF only, we can't let MCJIT to guess native platform as
  // it will be COFF on Win. We could've easily hardcoded data layouts here and
  // get away with it ('til next upgrade). To solve this problem once and for
  // all (hopefully) and avoid MCJIT assertion regarding DataLayout mismatch
  // between module's and the one produced via EngineBuilder in CPUCompiler.cpp
  // we'll perform the following trick:
  //
  // Create a dummy module with a correct target triple (-elf) and create
  // EngineBuilder from it. Then we set datalayout of our module to the correct
  // one produced by TargetMachine.

  std::unique_ptr<Module> dummyModule(new Module("empty", M.getContext()));
  dummyModule.get()->setTargetTriple(M.getTargetTriple());
  UpdateTargetTriple(dummyModule.get());
  EngineBuilder builder(std::move(dummyModule));
  auto TM =std::unique_ptr<TargetMachine>(builder.selectTarget());

  // That's how MCJIT does when being created.
  M.setDataLayout(TM->createDataLayout());
}
}
