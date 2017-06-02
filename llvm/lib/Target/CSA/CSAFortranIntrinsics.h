//===-- llvm/lib/Target/CSA/CSAFortranIntrinsics.h --------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration for the FortranIntrinsics pass, which
// converts certain function calls to intrinsics to make it possible to access
// the intrinsics in Fortran
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAFORTRANINTRINSICS_H
#define LLVM_LIB_TARGET_CSA_CSAFORTRANINTRINSICS_H

namespace llvm {
  class FunctionPass;
  FunctionPass *createFortranIntrinsics();
}

#endif
