/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "Obfuscation.h"

#include "llvm/InitializePasses.h"
#include "llvm/IR/Type.h"

#include "MetaDataApi.h"
#include "OCLPassSupport.h"
#include "CompilationUtils.h"

#include <algorithm>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

//
// Obfuscation
//

char Obfuscation::ID = 0;

Obfuscation::Obfuscation(): ModulePass(ID) {}

class ObfuscationBlockFunctor: public BlockFunctor {
  long m_counter;
public:
  ObfuscationBlockFunctor(): m_counter(0) {}

  virtual void operator ()(llvm::BasicBlock& BB) {
    BB.setName(llvm::Twine(""));

    for(llvm::BasicBlock::iterator it = BB.begin(), e = BB.end(); it != e;
        ++it) {
      if (!it->getType()->isVoidTy())
        it->setName(llvm::Twine(""));
    }
  }
};

struct TrivialFunctionFunctor: public FunctionFunctor {
  virtual void operator ()(llvm::Function& F) {
    ObfuscationBlockFunctor bFunctor;
    // Changing the name of the arguments of the function.
    llvm::Function::ArgumentListType &ArgList = F.getArgumentList();
    typedef llvm::Function::ArgumentListType::iterator ArgIter;
    int counter = 0;

    for (ArgIter it = ArgList.begin(), e=ArgList.end(); it != e; it++)
      it->setName(llvm::Twine(counter++));

    std::for_each(F.begin(), F.end(), bFunctor);
  }
};

bool Obfuscation::runOnModule(llvm::Module& M) {
  // Setting the metadata with the argument name to empty string (if exists).
  Intel::MetaDataUtils mdUtils(&M);
  for (Intel::MetaDataUtils::KernelsList::iterator it = mdUtils.begin_Kernels(),
                                                    e = mdUtils.end_Kernels();
      it != e;
      it++) {
      Intel::KernelMetaData *KM = it->get();
      for (size_t i=0; i<KM->size_ArgName(); i++)
        KM->setArgNameItem(i, "");
  }

  TrivialFunctionFunctor fFunctor;
  std::for_each(M.begin(), M.end(), fFunctor);

  mdUtils.save(M.getContext());
  return true;
}

OCL_INITIALIZE_PASS_BEGIN(Obfuscation, "", "", false, false)
OCL_INITIALIZE_PASS_END(Obfuscation, "module-obfuscation",
  "Changes variable names to be meaningless.",
  false, // Not CGF only pass.
  false  // Not an analysis pass.
)

}// End namespace intel

extern "C" {

llvm::ModulePass *createObfuscation() {
  return new intel::Obfuscation();
}

}

