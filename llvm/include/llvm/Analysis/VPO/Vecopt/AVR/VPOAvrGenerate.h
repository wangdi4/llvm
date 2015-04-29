//===----------- VPOAvrGenerate.h - Generates AVR nodes ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This analysis class is used to generate AVR nodes from LLVM IR and
// LoopOpt HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_GENERATE_H
#define LLVM_ANALYSIS_VPO_AVR_GENERATE_H

#include "llvm/Pass.h"
#include "llvm/InitializePasses.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOPasses.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtils.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"


namespace intel { // VPO vectorizer namespace

using namespace llvm;

class AVRGenerate : public FunctionPass {

private:

  /// The current function being analysed
  Function *Func;
  /// The avr tree generated from Func.
  AVRContainerTy AVRList;


public:

  static char ID; // Pass Identification
  AVRGenerate();
  //  ~AVRGenerate();

  bool runOnFunction(Function &F);
  void create();
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print();
  void buildBody(AVRFunction *AVRFunc);

};

}  // End VPO Vectorizer namespace


#endif // LLVM_ANALYSIS_VPO_AVR_GENERATE_H
