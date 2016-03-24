//===------ VPOPasses.h - Constructors for VPO analyses ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the VPO library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_PASSES_H
#define LLVM_ANALYSIS_VPO_PASSES_H

namespace llvm {

  class FunctionPass;

  /// createAVRGeneratePass - This creates a pass that generates AVRs needed
  /// for vectorization
  FunctionPass *createAVRGeneratePass();
}

#endif
