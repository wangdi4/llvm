//===------------------------------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOAvrGenerate.h -- Defines the analysis pass used to generate AVR nodes
//   from LLVM IR and HIR.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_AVR_GENERATE_H
#define LLVM_ANALYSIS_VPO_AVR_GENERATE_H

#include "llvm/Pass.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvrUtils.h"
#include "llvm/Analysis/VPO/Vecopt/AVR/VPOAvr.h"

namespace llvm { // LLVM Namespace

class DominatorTree;
class LoopInfo;

namespace vpo {  // VPO Vectorizer namespace

class IdentifyVectorCandidates;

class AVRGenerate : public FunctionPass {

private:

  /// The current function being analysed
  Function *Func;

  /// The avr tree generated from Func.
  AVRContainerTy AVRList;

  /// The AVR Loop currently being processed/generated.
  AVRLoop *AvrLoop;

  /// True when in AVR scalar stress testing mode
  bool ScalarStressTest;

  /// Identify Vector Candidates Pass
  IdentifyVectorCandidates *VC;  

  /// Dominator Tree
  DominatorTree *DT;

public:

  AVRGenerate();

  // Pass Identification
  static char ID;

  bool runOnFunction(Function &F);
  void create();
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print();
  void buildBody(AVRFunction *AVRFunc);
  void buildAvrForFunction();
  void buildAvrForVectorCandidates();
  AVR *preorderTravAvrBuild(BasicBlock *BB, AVR *InsertionPos);
  AVR *generateAvrInstSeqForBB(BasicBlock *BB, AVR *InsertionPos);

  /// \brief Code generation for AVRs. We have this under analysis for
  /// now. Clients call this from a transform pass. This will change
  /// and will move into transforms once we have AVR visitors. Returns 
  /// true if code was genreated from AVR.
  bool codeGen();

};

} // End VPO Vectorizer Namespace
} // End LLVM Namespace

#endif // LLVM_ANALYSIS_VPO_AVR_GENERATE_H
