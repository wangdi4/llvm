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
#include "llvm/Support/Debug.h"
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

  for (auto Name : MainNames) {
    if (GlobName.compare(Name) == 0)
      return true;
  }

  return false;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void WholeProgramUtils::AddSymbolResolution(llvm::StringRef SymbolName,
                                            unsigned Attributes) {

  WholeProgramReadSymbol Symbol(SymbolName, Attributes);
  SymbolsVector.push_back(Symbol);
}

void WholeProgramUtils::PrintSymbolsResolution() {

  unsigned SymbolsResolved = 0;
  unsigned SymbolsUnresolved = 0;

  for (auto Symbol : SymbolsVector) {
    dbgs() << "SYMBOL NAME: " << Symbol.getName() << "\n";
    dbgs() << "  RESULT:";
    dbgs() << (Symbol.isMain() ? " MAIN |" : "");
    dbgs() << (Symbol.isLinkerAddedSymbol() ? " LINKER ADDED SYMBOL |" : "");
    dbgs() << (Symbol.isResolvedByLinker() ? "" : " NOT") <<
                " RESOLVED BY LINKER \n\n";

    if (Symbol.isLinkerAddedSymbol() || Symbol.isResolvedByLinker())
      SymbolsResolved++;

    else
      SymbolsUnresolved++;
  }

  dbgs() << "SYMBOLS RESOLVED BY LINKER: " << SymbolsResolved << "\n";
  dbgs() << "SYMBOLS NOT RESOLVED BY LINKER: " << SymbolsUnresolved << "\n";
}
#endif // NDEBUG || LLVM_ENABLE_DUMP
