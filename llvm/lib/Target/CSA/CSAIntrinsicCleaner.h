//===-- llvm/lib/Target/CSA/CSAIntrinsicCleaner.h ---------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the CSAIntrinsicCleaner pass, which
// cleans up unused CSA intrinsics that shouldn't show up in the backend and
// detects iteration-local storage.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAINTRINSICCLEANER_H
#define LLVM_LIB_TARGET_CSA_CSAINTRINSICCLEANER_H

namespace llvm {
  class Pass;
  Pass* createCSAIntrinsicCleanerPass();
}

#endif
