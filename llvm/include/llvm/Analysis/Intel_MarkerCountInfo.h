#if INTEL_FEATURE_MARKERCOUNT
//==- llvm/Analysis/Intel_MarkerCountInfo.h - Marker Count Parser -*- C++-*-===//
//
// Copyright (C) 2016-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===-----------------------------------------------------------------------===//
//
// This file defines the APIs that are used to parse the marker count for
// functions in from a JSON file/string.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_MARKERCOUNTINFO_H
#define LLVM_ANALYSIS_MARKERCOUNTINFO_H

#include "llvm/ADT/StringRef.h"
#include <map>
#include <string>

namespace llvm {
namespace MarkerCount {
/// Parse the mark count from a JSON file and store the result into a map.
///
/// \param Map destination to store
/// \param MarkerCountKind the default marker count kind to use
/// \param File the JSON file to parse
void parseMarkerCountFile(std::map<std::string, unsigned> &Map,
                          unsigned MarkerCountKind, StringRef File);
} // namespace MarkerCount
} // namespace llvm

#endif
#endif // INTEL_FEATURE_MARKERCOUNT
