//===- Intel_FMACommon.cpp - Fused Multiply Add optimization --------------===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file defines the base classes used by two separate components:
//   1) Table-Gen generating efficient FMA patterns;
//   2) GlobalFMA optimization.
//
//===----------------------------------------------------------------------===//

#include "llvm/CodeGen/Intel_FMACommon.h"

namespace llvm {

/// FMADagCommon static fields.
const unsigned FMADagCommon::MaxNumOfNodesInDAG;
const unsigned FMADagCommon::MaxNumOfUniqueTermsInDAG;
const uint8_t FMADagCommon::TermZERO;
const uint8_t FMADagCommon::TermONE;
const uint8_t FMADagCommon::MaxTermIndex;

/// FMAExprSPCommon static fields.
const unsigned FMAExprSPCommon::MaxNumOfTermsInProduct;
const unsigned FMAExprSPCommon::MaxNumOfUniqueTermsInSP;
const uint8_t FMAExprSPCommon::TermZERO;
const uint8_t FMAExprSPCommon::TermONE;

} // namespace llvm
