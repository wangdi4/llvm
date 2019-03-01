//===- Version.cpp - Clang Version Number -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines several version-related utility functions for Clang.
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/Version.h"
#include "clang/Basic/LLVM.h"
#include "clang/Config/config.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

#ifdef HAVE_SVN_VERSION_INC
#  include "SVNVersion.inc"
#endif

namespace clang {

std::string getClangRepositoryPath() {
#if defined(CLANG_REPOSITORY_STRING)
  return CLANG_REPOSITORY_STRING;
#else
#ifdef SVN_REPOSITORY
  StringRef URL(SVN_REPOSITORY);
#else
  StringRef URL("");
#endif

  // If the SVN_REPOSITORY is empty, try to use the SVN keyword. This helps us
  // pick up a tag in an SVN export, for example.
  StringRef SVNRepository("$URL: https://iclsvnsc.sc.intel.com/ssg/dpd/icl/llorgcfe/cfe/trunk/lib/Basic/Version.cpp $");
  if (URL.empty()) {
    URL = SVNRepository.slice(SVNRepository.find(':'),
                              SVNRepository.find("/lib/Basic"));
  }

  // Strip off version from a build from an integration branch.
  URL = URL.slice(0, URL.find("/src/tools/clang"));

  // Trim path prefix off, assuming path came from standard cfe path.
  size_t Start = URL.find("cfe/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start + 4);

  return URL;
#endif
}

std::string getLLVMRepositoryPath() {
#ifdef LLVM_REPOSITORY
  StringRef URL(LLVM_REPOSITORY);
#else
  StringRef URL("");
#endif

  // Trim path prefix off, assuming path came from standard llvm path.
  // Leave "llvm/" prefix to distinguish the following llvm revision from the
  // clang revision.
  size_t Start = URL.find("llvm/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start);

  return URL;
}

std::string getClangRevision() {
#ifdef SVN_REVISION
  return SVN_REVISION;
#else
  return "";
#endif
}

std::string getLLVMRevision() {
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
}

std::string getClangFullRepositoryVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
  std::string Path = getClangRepositoryPath();
  std::string Revision = getClangRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return OS.str();
}

std::string getClangFullVersion() {
  return getClangToolFullVersion("clang");
}

std::string getClangToolFullVersion(StringRef ToolName) {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#if INTEL_CUSTOMIZATION
  OS << "icx (ICX) 2019.8.1.0";
#endif // INTEL_CUSTOMIZATION
  return OS.str();
}

std::string getClangFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
  OS << "Clang " CLANG_VERSION_STRING " " << getClangFullRepositoryVersion();
  return OS.str();
}

#if INTEL_CUSTOMIZATION
std::string getXMainFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef CLANG_VENDOR
  OS << CLANG_VENDOR;
#endif
#ifdef GNU_VERSION_STRING
  OS << "Intel(R) Clang Based C++, gcc " << GNU_VERSION_STRING << " mode";
#else
  OS << "Intel(R) Clang Based C++, clang " << CLANG_VERSION_STRING;
#endif // GNU_VERSION_STRING
  return OS.str();
}
#endif // INTEL_CUSTOMIZATION

} // end namespace clang
