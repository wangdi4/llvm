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

// This flag will be set to true by linker when it is linking an
// executable.
static bool LinkingExecutable = false;

// This flag will be set to true after the whole program has been read i.e. the
// linker was able to resolve each function symbol to either one of linked
// modules or dynamic libraries.
static bool WholeProgramRead = false;

// Set if the linker is generating an executable
void llvm::setLinkingExecutable(bool LinkingExe) {
  LinkingExecutable = LinkingExe;
}

// Return true if the linker is creating an executable, else return false.
bool llvm::getLinkingExecutable() {
  return LinkingExecutable;
}

// Set true if all symbols were resolved by the linker.
void llvm::setWholeProgramRead(bool ProgramRead) {
  WholeProgramRead = ProgramRead;
}

// Return true if all symbols were resolved by the linker
bool llvm::getWholeProgramRead() {
  return WholeProgramRead;
}

// Return true if the symbol is a special symbol added by
// the linker, else return false.
bool llvm::isLinkerAddedSymbol(llvm::StringRef SymbolName) {
  return llvm::StringSwitch<bool>(SymbolName)
      .Cases("__ehdr_start",
             "__executable_start",
             "__dso_handle",
             true)
      .Default(false);
}
