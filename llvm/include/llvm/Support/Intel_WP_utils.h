//===------- Intel_WP_utils.h - Whole program analysis utilities -*------===//
//
// Copyright (C) 2016-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// Whole Program Analysis Utilities
//===----------------------------------------------------------------------===//
//
#ifndef LLVM_SUPPORT_INTELWP_UTILS_H
#define LLVM_SUPPORT_INTELWP_UTILS_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Pass.h"

namespace llvm {

class WholeProgramUtils {
private:
  // This flag will be set to true by linker when it is linking an
  // executable.
  bool LinkingExecutable = false;

  // This flag will be set to true after the whole program has been read i.e.
  // the linker was able to resolve each function symbol to either one of
  // linked modules or dynamic libraries.
  bool WholeProgramRead = false;

  // True if the LTO process finds that all symbols are inside the LTO unit.
  // The only symbols that will be treated as externals are main and those
  // that are in the RuntimeLibcalls table.
  bool HiddenVisibility = false;

  // SetVector for storing the symbols that are visible to regular objects.
  // These symbols might have IR in the summary section, but the LTO
  // visibility analysis found that there might be a none-LTO unit that
  // are accessing them.
  SetVector<StringRef> VisibleSymbolsVector;

public:
  WholeProgramUtils() {}

  // Return true if the symbol is a special symbol added by
  // the linker, else return false.
  bool isLinkerAddedSymbol(llvm::StringRef SymbolName);

  // Set true if the linker resolved all symbols, else set false.
  void setWholeProgramRead(bool ProgramRead);

  // Return true if the linker resolved all symbols, else return false.
  bool getWholeProgramRead();

  // Set if the linker is generating an executable.
  void setLinkingExecutable(bool LinikingExe);

  // Return true if the linker is creating an executable, else return false.
  bool getLinkingExecutable();

  // Return true if the input GlobName is a form of main,
  // else return false.
  bool isMainEntryPoint(llvm::StringRef GlobName);

  // Store if all symbols have hidden visibility. Called during LTO
  // symbols resolution.
  void setVisibilityHidden(bool AllSymbolsHidden);

  // Return if the LTO process could internalize all symbols
  bool getHiddenVisibility();

  // Store the input symbol name in the VisibleSymbolsVector. These
  // symbols will be printed during the whole program analysis trace.
  void storeVisibleSymbols(StringRef SymbolName);

  // Print the symbols that weren't marked as hidden during LTO
  void dumpVisibleSymbols();

};  // WholeProgramUtils

extern WholeProgramUtils WPUtils;

} // llvm

#endif // LLVM_SUPPORT_INTELWP_UTILS_H
