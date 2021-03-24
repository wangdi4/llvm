//===--- DTransOPOptBase.cpp - Base class for DTrans Transforms -----==//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file provides the base classes for DTrans Transformations that provide
// the common functionality needed for rewriting dependent data types and
// functions that change as the result of DTrans modifying a structure
// definition. This is to work with an opaque pointer representation.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/DTransOPOptBase.h"

#define DEBUG_TYPE "dtransop-optbase"

namespace llvm {
namespace dtransOP {

bool DTransOPOptBase::run(Module &M) {
  // TODO: Implement the calls to collect types and perform the transformation.
  return false;
}
} // namespace dtransOP
} // namespace llvm
