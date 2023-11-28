//===-- CSAMCExpr.cpp - CSA specific MC expression classes ------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of printing out register names for CSA
// LICs.
//
//===----------------------------------------------------------------------===//

#include "CSAMCTargetExpr.h"
#include "llvm/MC/MCContext.h"

using namespace llvm;

const CSAMCExpr *CSAMCExpr::create(unsigned regno, StringRef name,
                                   MCContext &Ctx) {
  return new (Ctx) CSAMCExpr(regno, name);
}

void CSAMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  OS << "%" << Name;
}
