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
#include "llvm/Support/Intel_WP_utils.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {
  WholeProgramUtils WPUtils;
} //llvm

// Set if the linker is generating an executable
void WholeProgramUtils::setLinkingExecutable(bool LinkingExe) {
  LinkingExecutable = LinkingExe;
}

// Return true if the linker is creating an executable, else return false.
bool WholeProgramUtils::getLinkingExecutable() {
  return LinkingExecutable;
}

// Set true if all symbols were resolved by the linker.
void WholeProgramUtils::setWholeProgramRead(bool ProgramRead) {
  WholeProgramRead = ProgramRead;
}

// Return true if all symbols were resolved by the linker
bool WholeProgramUtils::getWholeProgramRead() {
  return WholeProgramRead;
}

// Return true if the symbol is a special symbol added by
// the linker, else return false.
bool WholeProgramUtils::isLinkerAddedSymbol(llvm::StringRef SymbolName) {
  return llvm::StringSwitch<bool>(SymbolName)
      .Cases("__ehdr_start",
             "__executable_start",
             "__dso_handle",
             true)
      .Default(false);
}

// Return true if the input StringRef represents any form of
// main. Else return false.
bool WholeProgramUtils::isMainEntryPoint(llvm::StringRef GlobName) {

  return llvm::StringSwitch<bool>(GlobName)
      .Cases("main",
             "MAIN__",
             "wmain",
             "WinMain",
             "wWinMain",
             "DllMain",
             true)
      .Default(false);

}

// Store if all symbols have hidden visibility. Called during LTO
// symbols resolution.
void WholeProgramUtils::setVisibilityHidden(bool AllSymbolsHidden) {
  HiddenVisibility = AllSymbolsHidden;
}

// Return if the LTO process could internalize all symbols in the module
bool WholeProgramUtils::getHiddenVisibility(){
  return HiddenVisibility;
}

// Store the input symbol name in the VisibleSymbolsVector. These
// symbols will be printed during the whole program analysis trace.
void WholeProgramUtils::storeVisibleSymbols(StringRef SymbolName) {
  VisibleSymbolsVector.insert(SymbolName);
}

// Print the symbols that weren't marked as hidden during LTO
void WholeProgramUtils::dumpVisibleSymbols() {
  errs() << "  VISIBLE OUTSIDE LTO: " << VisibleSymbolsVector.size() << "\n";
  for (StringRef SymbolName : VisibleSymbolsVector) {
    errs() << "      " << SymbolName << "\n";
  }
}
