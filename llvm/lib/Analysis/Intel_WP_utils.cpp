//===------- Intel_WP_utils.cpp - Whole Program Analysis Utilities -*-----===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file contains the utilities that will be shared between the compiler
// and the linkers to achieve whole program.
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/Intel_WP.h"

using namespace llvm;

// TODO: This function will be used as a wrapper function until LLD
// points to the correct version of whole program utilities.

// Set if the linker is generating an executable
void llvm::setLinkingExecutable(bool LinkingExe) {
  WPUtils.setLinkingExecutable(LinkingExe);
}
