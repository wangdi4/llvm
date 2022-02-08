//===----- OptReportSupport.cpp - Utils to support emitters -*- C++ -*------==//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements a set of routines to support various emitters
// of loopopt, vectorizer and OpenMP regions related Optimization Reports.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringMap.h"
#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include <string>

namespace llvm {
namespace OptReportSupport {

/// Returns a string of bytes representing the given \p OptReport
/// in binary form, which may be emitted directly into the binary.
std::string formatBinaryStream(OptReport OptReport);

/// Returns true if Protobuf-based binary opt-report encoding feature is
/// enabled.
bool isProtobufBinOptReportEnabled();

/// Type to represent a map of LoopOptReports that should be encoded in
/// Protobuf-based binary opt-report. Every loop is identified by a unique
/// anchor ID which is used as key for the map.
using OptRptAnchorMapTy = StringMap<OptReport>;

/// Returns a string of bytes representing Protobuf-based binary encoding of
/// given map of opt-reports in \p OptRptAnchorMapTy. Binary opt-report is
/// generated for version \p MajorVer and \p MinorVer. This encoded binary data
/// may be emitted directly in outgoing object file.
std::string generateProtobufBinOptReport(OptRptAnchorMapTy &OptRptAnchorMap,
                                         unsigned MajorVer, unsigned MinorVer);

} // namespace OptReportSupport
} // namespace llvm
