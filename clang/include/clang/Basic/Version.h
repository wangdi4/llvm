//===- Version.h - Clang Version Number -------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines version macros and version-related utility functions
/// for Clang.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_VERSION_H
#define LLVM_CLANG_BASIC_VERSION_H

#include "clang/Basic/Version.inc"
#include "llvm/ADT/StringRef.h"

#if INTEL_CUSTOMIZATION
  // CQ374831: define GNU_VERSION_STRING

#if defined __GNUC__ && defined __GNUC_MINOR__

#define GNU_MAKE_VERSION_STRING2(X) #X

#ifdef __GNUC_PATCHLEVEL__
/// A string that describes the gnu version number, e.g., "1.0".
#define GNU_MAKE_VERSION_STRING(X,Y,Z) GNU_MAKE_VERSION_STRING2(X.Y.Z)
#define GNU_VERSION_STRING \
  GNU_MAKE_VERSION_STRING(__GNUC__,__GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#else
#define GNU_MAKE_VERSION_STRING(X,Y) GNU_MAKE_VERSION_STRING2(X.Y)
#define GNU_VERSION_STRING \
  GNU_MAKE_VERSION_STRING(__GNUC__,__GNUC_MINOR__)
#endif // __GNUC_PATCHLEVEL__
#endif // defined __GNUC__ && defined __GNUC_MINOR__
#endif // INTEL_CUSTOMIZATION

namespace clang {
  /// Retrieves the repository path (e.g., Subversion path) that
  /// identifies the particular Clang branch, tag, or trunk from which this
  /// Clang was built.
  std::string getClangRepositoryPath();

  /// Retrieves the repository path from which LLVM was built.
  ///
  /// This supports LLVM residing in a separate repository from clang.
  std::string getLLVMRepositoryPath();

  /// Retrieves the repository revision number (or identifier) from which
  /// this Clang was built.
  std::string getClangRevision();

  /// Retrieves the repository revision number (or identifier) from which
  /// LLVM was built.
  ///
  /// If Clang and LLVM are in the same repository, this returns the same
  /// string as getClangRevision.
  std::string getLLVMRevision();

  /// Retrieves the full repository version that is an amalgamation of
  /// the information in getClangRepositoryPath() and getClangRevision().
  std::string getClangFullRepositoryVersion();

  /// Retrieves a string representing the complete clang version,
  /// which includes the clang version number, the repository version,
  /// and the vendor tag.
  std::string getClangFullVersion();

  /// Like getClangFullVersion(), but with a custom tool name.
  std::string getClangToolFullVersion(llvm::StringRef ToolName);

  /// Retrieves a string representing the complete clang version suitable
  /// for use in the CPP __VERSION__ macro, which includes the clang version
  /// number, the repository version, and the vendor tag.
  std::string getClangFullCPPVersion();

#if INTEL_CUSTOMIZATION
  std::string getICXVersionNumber();

  std::string getICXVersionString();

  /// Version string for DPC++
  std::string getDPCPPVersionString();

  /// Version string for xmain: cq374831
  std::string getXMainFullCPPVersion();
#endif // INTEL_CUSTOMIZATION
}

#endif // LLVM_CLANG_BASIC_VERSION_H
