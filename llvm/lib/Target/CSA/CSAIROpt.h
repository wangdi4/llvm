//===-- llvm/lib/Target/CSA/CSAIROpt.h ---------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the CSAIRReductionOpt pass
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAIROPT_H
#define LLVM_LIB_TARGET_CSA_CSAIROPT_H

namespace llvm {
  class Pass;
  Pass* createCSAIRReductionOptPass();
}

#endif
