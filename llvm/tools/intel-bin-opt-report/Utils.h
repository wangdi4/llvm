//===- Utils.h - Binary opt-report reader tool utils-----------*- C++ -*---===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements some utililties used in binary opt-report reader tool.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_INTEL_BIN_OPT_REPORT_UTILS_H
#define LLVM_TOOLS_INTEL_BIN_OPT_REPORT_UTILS_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/WithColor.h"

#define ITT_TABLE_VERSION_MAJOR(v)    (((v) >> 8) & 0xff)
#define ITT_TABLE_VERSION_MINOR(v)    ((v) & 0xff)

namespace llvm {

static void error(Twine Msg, StringRef InputFileName) {
  // Flush the standard output to print the error at a proper place.
  fouts().flush();
  WithColor::error(errs(), "intel-bin-opt-report")
      << InputFileName << " : " << Msg << "\n";
  exit(1);
}

static void error(Error E, StringRef InputFileName) {
  // Flush the standard output to print the error at a proper place.
  fouts().flush();
  std::string Buf;
  raw_string_ostream OS(Buf);
  logAllUnhandledErrors(std::move(E), OS);
  OS.flush();
  WithColor::error(errs(), "intel-bin-opt-report")
      << InputFileName << " : " << Buf << "\n";
  exit(1);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
static void dumpRawByteStream(std::string &Stream, raw_ostream &OS) {
  OS << "Length of stream: " << Stream.size() << "\n";
  OS << "Raw bytestream: ";

  for (unsigned I = 0; I < Stream.size(); ++I) {
    OS.write_hex((int)Stream[I]);
    OS << " ";
  }

  OS << "\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

} // namespace llvm

#endif // LLVM_TOOLS_INTEL_BIN_OPT_REPORT_UTILS_H
