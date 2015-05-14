#ifndef LLVM_TRANSFORMS_VPO_VECOPT_PASSES_H
#define LLVM_TRANSFORMS_VPO_VECOPT_PASSES_H

namespace llvm {

  class ModulePass;

  /// createVPOVectorizer - This creates the VPO vectorizer pass 
  ModulePass *createVPOVectorizerPass();
}

#endif

