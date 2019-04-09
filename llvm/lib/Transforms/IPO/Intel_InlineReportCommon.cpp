//===--------- Intel_InlineReportCommon.cpp - Inlining Reporti utils  ----===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements inline report utils.
//
//===----------------------------------------------------------------------===//
//
#include "llvm/Transforms/IPO/Intel_InlineReportCommon.h"

using namespace llvm;

///
/// \brief Print 'indentCount' indentations
///
void llvm::printIndentCount(unsigned indentCount) {
  llvm::errs().indent(indentCount * 3);
}
