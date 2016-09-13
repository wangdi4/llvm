/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "CompilationUtils.h"
#include "Materialize.h"
#include "MetaDataApi.h"
#include "OCLPassSupport.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Target/TargetMachine.h"

#include <cstdio>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";          // Used for RH64/SLES64.
const char *PC_LIN32 = "i686-pc-linux";            // Used for Android.
const char *PC_WIN32 = "i686-pc-win32-msvc-elf";   // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32-msvc-elf"; // Win 64 bit.

// Basic block functors, to be applied on each block in the module.
// 1. Replaces calling conventions in calling sites.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  void operator()(llvm::BasicBlock &BB) {
    llvm::BasicBlock::iterator b = BB.begin(), e = BB.end();
    while (e != b) {
      if (llvm::CallInst *CI = llvm::dyn_cast<llvm::CallInst>(&*b)) {
        if ((llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
            (llvm::CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
          CI->setCallingConv(llvm::CallingConv::C);
          m_isChanged = true;
        }
      }
      ++b;
    }
  }
};

// Function functor, to be applied for every function in the module.
// 1. Delegates call to basic-block functors.
// 2. Replaces calling conventions of function declarations.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  void operator()(llvm::Function &F) {
    MaterializeBlockFunctor bbMaterializer;
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      m_isChanged = true;
    }
    std::for_each(F.begin(), F.end(), bbMaterializer);
    m_isChanged |= bbMaterializer.isChanged();
  }
};

//
// SpirMaterializer
//

char SpirMaterializer::ID = 0;

SpirMaterializer::SpirMaterializer() : ModulePass(ID) {}

const char *SpirMaterializer::getPassName() const {
  return "spir materializer";
}

bool SpirMaterializer::runOnModule(llvm::Module &M) {
  bool Ret = false;

  MaterializeFunctionFunctor fMaterializer;
  // Take care of calling conventions
  std::for_each(M.begin(), M.end(), fMaterializer);

  return Ret || fMaterializer.isChanged();
}

OCL_INITIALIZE_PASS_BEGIN(SpirMaterializer, "", "", false, false)
OCL_INITIALIZE_PASS_END(SpirMaterializer, "spir-materializer",
  "Prepares SPIR modules for BE consumption.",
  false, // Not CGF only pass.
  false  // Not an analysis pass.
)

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
  TargetMachine *TM = builder.selectTarget();

  // That's how MCJIT does when being created.
  M.setDataLayout(TM->createDataLayout());
}
}
