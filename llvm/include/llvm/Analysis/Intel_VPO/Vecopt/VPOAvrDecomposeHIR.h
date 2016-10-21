//===-- VPOAvrGenerate.h ----------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the analysis pass used to decompose a complex AVRValuesHIR
/// into a sub-expression tree with simpler AVRExpressionHIRs and AVRValuesHIRs.
/// The sub-expression tree generated is an AVRExpressionHIR that may have other
/// AVRExpressionHIRs or AVRValueHIRs as operands.
/// The resulting sub-expression tree is stored in the field DecompTree of the
/// original AVRValueHIR. 
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H
#define LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOAvrGenerate.h"

using namespace llvm::loopopt;

namespace llvm { // LLVM Namespace
namespace vpo { // VPO Vectorizer namespace

class AVRDecomposeHIR : public FunctionPass {

protected:

  AVRGenerateHIR *AVRG;

public:
  static char ID;

  AVRDecomposeHIR();

  bool runOnFunction(Function &F);
  bool runOnAvr(AVR *ANode);
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void print(raw_ostream &OS, unsigned Depth = 1,
             VerbosityLevel VLevel = PrintBase) const;
  void dump(VerbosityLevel VLevel = PrintBase) const;
  void releaseMemory();
};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_DECOMPOSE_H
