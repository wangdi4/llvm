//===- LoopOptReportSupport.cpp - Utils to support emitters -*- C++ -*------==//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of routines to support various emitters
// of loopopt and vectorizer related Loop Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include <string>
#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"

namespace llvm {
namespace LoopOptReportSupport {

/// Returns a string of bytes representing the given \p OptReport
/// in binary form, which may be emitted directly into the binary.
std::string formatBinaryStream(LoopOptReport OptReport);

} // namespace LoopOptReportSupport
} // namespace llvm
