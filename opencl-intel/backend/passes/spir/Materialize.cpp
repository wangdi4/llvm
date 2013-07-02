/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Materialize.h"
#include <cstdio>

#include "llvm/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/InitializePasses.h"

#include "gen/MetaDataApi.h"
#include "OCLPassSupport.h"

using namespace llvm;
using namespace Intel;

namespace intel {

// Supported target triple.
const char *PC_LIN64 = "x86_64-pc-linux"; //used for RH64/SLES64
const char *PC_WIN32 = "i686-pc-win32";   //Win 32 bit
const char *PC_WIN64 = "x86_64-pc-win32"; //Win 64 bit

//Native data layout details for each supported Triple
const char *Win32Natives = "-a0:0:64-f80:32:32-n8:16:32-S32";
const char *Lin32Natives = "-a0:0:64-f80:32:32-n8:16:32-S128";
const char *X86_64Natives= "-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128";

// Base class for all functors, which supports immutability query.
class AbstractFunctor{
protected:
  bool IsChanged;
public:
  AbstractFunctor(): IsChanged(false){}

  virtual ~AbstractFunctor() = 0;

  bool isChanged()const{
    return IsChanged;
  }
};

AbstractFunctor::~AbstractFunctor(){}

// Basic block functors, to be applied on each block in the module.
// 1. Replaces calling conventions in calling sites.
class BBFunctor: public AbstractFunctor {
public:
  void operator ()(llvm::BasicBlock& BB){
    llvm::BasicBlock::iterator b = BB.begin(), e = BB.end();
    while (e != b){
      if(llvm::CallInst *CI=llvm::dyn_cast<llvm::CallInst>(&*b)){
        if (llvm::CallingConv::SPIR_FUNC == CI->getCallingConv()){
          CI->setCallingConv(llvm::CallingConv::C);
          IsChanged = true;
        }
      }
      ++b;
    }
  }
};

// Function functor, to be applied for every function in the module.
// 1. Delegates call to basic-block functors.
// 2. Replaces calling conventions of function declarations.
class FunctionFunctor: public AbstractFunctor {
public:
  void operator ()(llvm::Function& F){
    BBFunctor bbMaterializer;
    llvm::CallingConv::ID CConv = F.getCallingConv();
    if (llvm::CallingConv::SPIR_FUNC == CConv ||
        llvm::CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(llvm::CallingConv::C);
      IsChanged = true;
    }
    std::for_each(F.begin(), F.end(), bbMaterializer);
    IsChanged |= bbMaterializer.isChanged();
  }
};

//
// SpirMaterializer
//

char SpirMaterializer::ID = 0;

Maybe<std::string> SpirMaterializer::getCrossTriple(llvm::Module& M) {
  const char*const CROSS_SWITCH = "-target-triple";
  MetaDataUtils mdUtils(&M);
  MetaDataUtils::CompilerOptionsList::iterator it = mdUtils.begin_CompilerOptions();
  MetaDataUtils::CompilerOptionsList::iterator e  = mdUtils.end_CompilerOptions();

  // Iterating the compilation option, in search on 'target-triple'.
  while(it != e) {
    if (!(*it)->isvalueHasValue()) {
      ++it;
      continue;
    }
    if (CROSS_SWITCH == (*it++)->getvalue()) {
      assert(it != e && "empty target triple");
      return Maybe<std::string>((*it)->getvalue());
    }
  }

  return Maybe<std::string>::null();
}


SpirMaterializer::SpirMaterializer(): ModulePass(ID) {
}

const char* SpirMaterializer::getPassName() const {
  return "spir materializer";
}

bool SpirMaterializer::runOnModule(llvm::Module& M) {
  bool Ret = false;

  FunctionFunctor fMaterializer;
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
  // Find out whether cross-compilation is involved. If so, we need to take the
  // triple from the metadata, not the one of the host.
  const char* NativeSuffix = 0;
  intel::Maybe<std::string> CrossTriple = intel::SpirMaterializer::getCrossTriple(M);
  if (!CrossTriple.isNull())
    Triple = CrossTriple.value();
  else {
    Triple =
#if defined(_M_X64)
     intel::PC_WIN64;
#elif defined(__LP64__)
     intel::PC_LIN64;
#else
     intel::PC_WIN32;
#endif
  }
  M.setTargetTriple(Triple);

  // Adjusting the datalayout, to fit the new (materialized) triple.
  if (Triple == intel::PC_WIN64 || Triple == intel::PC_LIN64)
    NativeSuffix = intel::X86_64Natives;
  else if (Triple == intel::PC_WIN32)
    NativeSuffix = intel::Win32Natives;
  else {
    //used for Android
    assert(Triple == "i686-pc-linux" && "Unsupported Triple in cross compilation");
    NativeSuffix = intel::Lin32Natives;
  }
  std::string strDataLayout = M.getDataLayout();
  strDataLayout.append(NativeSuffix);
  M.setDataLayout(llvm::StringRef(strDataLayout));
}

}

