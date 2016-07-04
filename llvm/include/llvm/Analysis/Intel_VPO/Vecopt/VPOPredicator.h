//===-- VPODriver.h ---------------------------------------------*- C++ -*-===//
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

namespace llvm {

namespace vpo {

class SESERegion;

class VPOPredicator {

public:

  VPOPredicator();

  void runOnAvr(AVRLoop* ALoop);

private:

  void predicateLoop(AVRLoop* ALoop);

  void handleSESERegion(const SESERegion *Region, AvrCFGBase* CFG);

  void predicate(AVRBlock* Entry);

  void removeCFG(AVRBlock* Entry);
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_TRANSFORMS_VPO_VPOPREDICATOR_H
