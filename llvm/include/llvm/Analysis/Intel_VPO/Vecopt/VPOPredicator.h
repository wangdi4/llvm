//===-- VPOPredicator.h -----------------------------------------*- C++ -*-===//
//
//   Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//   Source file:
//   ------------
//   VPOPredicator.h -- Defines the VPO Predication analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPOPREDICATOR_H
#define LLVM_TRANSFORMS_VPO_VPOPREDICATOR_H

#include "llvm/Analysis/Intel_VPO/Vecopt/VPOCFG.h"
#include <stack>

namespace llvm {

namespace vpo {

class SESERegion;

class VPOPredicatorBase {


public:
  /// Incoming AVR
  AVRGenerateBase *AVRG = nullptr;
  void runOnAvr(AVRLoop *ALoop);

protected:
  // We don't allow the base class to be constructed
  VPOPredicatorBase() {}

  bool runOnFunction(Function &F);

private:
  void predicateLoop(AVRLoop *ALoop);
  void handleSESERegion(const SESERegion *Region, AvrCFGBase *CFG);
  void predicate(AVRBlock *Entry);
  void removeCFG(AVRBlock *Entry);
};

class VPOPredicator : public VPOPredicatorBase, public FunctionPass {

public:
  /// Pass Identification
  static char ID;

  VPOPredicator(); 

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F);
};

class VPOPredicatorHIR : public VPOPredicatorBase, public FunctionPass {

public:
   /// Pass Identification
  static char ID;

  VPOPredicatorHIR(); 

  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnFunction(Function &F); 
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPOPREDICATOR_H
