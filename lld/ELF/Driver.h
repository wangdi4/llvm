//===- Driver.h -------------------------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021-2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLD_ELF_DRIVER_H
#define LLD_ELF_DRIVER_H

#include "LTO.h"
#include "lld/Common/LLVM.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Option/ArgList.h"

namespace lld::elf {
class InputFile;

extern std::unique_ptr<class LinkerDriver> driver;

class LinkerDriver {
public:
  void linkerMain(ArrayRef<const char *> args);
  void addFile(StringRef path, bool withLOption);
  void addLibrary(StringRef name);

private:
  void createFiles(llvm::opt::InputArgList &args);
  void inferMachineType();
  void link(llvm::opt::InputArgList &args);
  template <class ELFT> void compileBitcodeFiles(bool skipLinkedOutput);

  // True if we are in --whole-archive and --no-whole-archive.
  bool inWholeArchive = false;

  // True if we are in --start-lib and --end-lib.
  bool inLib = false;

  // For LTO.
  std::unique_ptr<BitcodeCompiler> lto;

  std::vector<InputFile *> files;

#if INTEL_CUSTOMIZATION
  // Helper function for finding the ELF target used for GNU LTO files and
  // invoke doGNULTOLinking.
  void finalizeGNULTO(llvm::SmallVectorImpl<InputFile *> &InputGNULTOFiles,
                      bool isLazyFile);

  // Pass to g++ the input vector of GNU LTO files in order to do LTO and
  // build a temporary object. Then collect the ELF object generated and
  // add it to the linking process either as a regular object file or
  // lazy object (archive members).
  template <class ELFT> void
      doGNULTOLinking(llvm::SmallVectorImpl<InputFile *> &InputGNULTOFiles,
                      bool isLazyFile);
#endif // INTEL_CUSTOMIZATION

public:
  SmallVector<std::pair<StringRef, unsigned>, 0> archiveFiles;
};

// Parses command line options.
class ELFOptTable : public llvm::opt::OptTable {
public:
  ELFOptTable();
  llvm::opt::InputArgList parse(ArrayRef<const char *> argv);
};

// Create enum with OPT_xxx values for each option in Options.td
enum {
  OPT_INVALID = 0,
#define OPTION(_1, _2, ID, _4, _5, _6, _7, _8, _9, _10, _11, _12) OPT_##ID,
#include "Options.inc"
#undef OPTION
};

void printHelp();
std::string createResponseFile(const llvm::opt::InputArgList &args);

llvm::Optional<std::string> findFromSearchPaths(StringRef path);
llvm::Optional<std::string> searchScript(StringRef path);
llvm::Optional<std::string> searchLibraryBaseName(StringRef path);
llvm::Optional<std::string> searchLibrary(StringRef path);

} // namespace lld::elf

#endif
