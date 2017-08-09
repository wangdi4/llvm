//===-- llvm/lib/Target/CSA/CSALoopIntrinsicExpander.h ----------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the CSALoopIntrinsicExpander pass,
// which expands llvm.csa.parallel.loop intrinsics into their corresponding
// region/section representations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSALOOPINTRINSICEXPANDER_H
#define LLVM_LIB_TARGET_CSA_CSALOOPINTRINSICEXPANDER_H

namespace llvm {
  class Pass;
  Pass* createCSALoopIntrinsicExpanderPass();
}

#endif
