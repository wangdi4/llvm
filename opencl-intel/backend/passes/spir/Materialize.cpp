/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Materialize.h"
#include <cstdio>

#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Target/TargetMachine.h"

#include "MetaDataApi.h"
#include "OCLPassSupport.h"
#include "CompilationUtils.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";       // Used for RH64/SLES64.
const char *PC_LIN32 = "i686-pc-linux";         // Used for Android.
const char *PC_WIN32 = "i686-pc-win32-msvc";    // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32";       // Win 64 bit.

// Command line option used for cross compilation.
const char *CROSS_SWITCH = "-target-triple";

// Basic block functors, to be applied on each block in the module.
// 1. Replaces calling conventions in calling sites.
class MaterializeBlockFunctor: public BlockFunctor {
public:
  void operator ()(llvm::BasicBlock& BB){
    llvm::BasicBlock::iterator b = BB.begin(), e = BB.end();
    while (e != b){
      if(llvm::CallInst *CI=llvm::dyn_cast<llvm::CallInst>(&*b)){
        if ((llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
            (llvm::CallingConv::SPIR_KERNEL == CI->getCallingConv())){
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
class MaterializeFunctionFunctor: public FunctionFunctor {
public:
  void operator ()(llvm::Function& F){
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

SpirMaterializer::SpirMaterializer(): ModulePass(ID) {
}

const char* SpirMaterializer::getPassName() const {
  return "spir materializer";
}

bool SpirMaterializer::runOnModule(llvm::Module& M) {
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
llvm::ModulePass *createSpirMaterializer(){
  return new intel::SpirMaterializer();
}

void  materializeSpirDataLayout(llvm::Module& M) {
  llvm::StringRef Triple(M.getTargetTriple());
  if (!Triple.startswith("spir"))
    return;

  // Replace the triple with that of the actual host, in case the triple is SPIR.
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

  // Adjusting the datalayout, to fit the new (materialized) triple.
  EngineBuilder builder;
  TargetMachine* TM = builder.selectTarget(); // guess native
  // That's how MCJIT does when being created.
  M.setDataLayout(TM->createDataLayout());
}

}

