//===------- Intel_WP_utils.h - Whole program analysis utilities -*------===//
//
// Copyright (C) 2016 Intel Corporation. All rights reserved.
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

#include <vector>

namespace llvm {

// Helper class to store the information collected for each symbol during
// whole program read analysis.
class WholeProgramReadSymbol {
private:

  // Symbol name
  StringRef SymbolName;

  // Attributes found in the symbol
  unsigned Attributes = 0;

public:

  WholeProgramReadSymbol(StringRef &SymbolName, unsigned Attributes) :
                         SymbolName(SymbolName), Attributes(Attributes) {}

  // Attributes:

  // Is main
  static const unsigned AttrMain = 1;

  // Symbol was added by the linker
  static const unsigned AttrLinkerAdded = 1 << 1;

  // Symbol was resolved by the linker
  static const unsigned AttrResolved = 1 << 2;

  // Symbol was set as dynamic exported by the linker
  static const unsigned AttrExportDynamic = 1 << 3;

  // Return the symbol's name
  StringRef getName() { return SymbolName; }

  // Return true if the symbol is main
  bool isMain() { return Attributes & AttrMain; }

  // Return true if the symbol was added by the linker
  bool isLinkerAddedSymbol() { return Attributes & AttrLinkerAdded; }

  // Return true if the symbol was resolved by the linker
  bool isResolvedByLinker() { return Attributes & AttrResolved; }

  // Return true if the symbol was set as dynamic exported by the linker
  bool isExportDynamic() { return Attributes & AttrExportDynamic; }
}; // WholeProgramReadSymbol

class WholeProgramUtils {
private:
  // This flag will be set to true by linker when it is linking an
  // executable.
  bool LinkingExecutable = false;

  // This flag will be set to true after the whole program has been read i.e.
  // the linker was able to resolve each function symbol to either one of
  // linked modules or dynamic libraries.
  bool WholeProgramRead = false;

  // SetVector used to store the symbols resolution found by the linker.
  std::vector<WholeProgramReadSymbol> SymbolsVector;

  SmallVector<StringRef, 6> MainNames = { "main", "MAIN__", "wmain", "WinMain",
      "wWinMain", "DllMain" };

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

  auto &getSymbolsResolution() const { return SymbolsVector; }

  // Return the possible names that "main" can have.
  // NOTE: we don't return the Function* that points to "main" from here.
  // The issue is that this utility file is used across multiple applications
  // like LLVM and LLD. It can mix components, which is against the coding
  // standards. If you need to collect "main" the use
  // WholeProgramInfo::getMainFunction.
  SmallVector<StringRef, 6> getMainNames() { return MainNames; }

  // Insert a new symbol in the SymbolsVector
  void AddSymbolResolution(llvm::StringRef SymbolName, unsigned Attributes);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // Print the symbol resultion
  void PrintSymbolsResolution();
#endif // NDEBUG || LLVM_ENABLE_DUMP
};  // WholeProgramUtils

} // llvm

#endif // LLVM_SUPPORT_INTELWP_UTILS_H
