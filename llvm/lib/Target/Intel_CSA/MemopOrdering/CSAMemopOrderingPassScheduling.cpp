//===-- CSAMemopOrderingPassScheduling.cpp ----------------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file implements functions that handle scheduling memop ordering passes
/// for CSA.
///
///===---------------------------------------------------------------------===//

#include "../CSA.h"

#include "CSALinearMemopOrdering.h"
#include "CSAMemopOrdering.h"
#include "CSARaceModeMemopOrdering.h"

using namespace llvm;

enum MemopOrderingMode { standard, linear, race };

static cl::opt<MemopOrderingMode> OrderingMode{
  "csa-memop-ordering-mode", cl::Hidden,
  cl::desc("CSA Specific: Specify the memory ordering strategy to use."),
  cl::values(
    clEnumVal(
      standard,
      "The standard memop ordering pass; generally the best one to use."),
    clEnumVal(linear, "Ensures that no memory operation starts before the one "
                      "before it finishes. Slow but correct."),
    clEnumVal(
      race, "The opposite of linear ordering: memops are not ordered with "
            "anything except the function start and end. Not generally correct "
            "and prone to data races, but sometimes useful for experiments.")),
  cl::init(standard)};

void llvm::initializeCSAMemopOrderingPasses(PassRegistry &PR) {
  initializeCSALinearMemopOrderingPass(PR);
  initializeCSARaceModeMemopOrderingPass(PR);
  initializeCSAMemopOrderingPass(PR);
}

Pass *llvm::createCSAMemopOrderingPass(const CSATargetMachine &TM) {
  switch (OrderingMode) {
  case standard:
    return createStandardCSAMemopOrderingPass(&TM);
  case linear:
    return new CSALinearMemopOrdering;
  case race:
    return new CSARaceModeMemopOrdering;
  default:
    llvm_unreachable("Unknown memory ordering mode?");
  }
}
