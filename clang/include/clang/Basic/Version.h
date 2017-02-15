//===- Version.h - Clang Version Number -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines version macros and version-related utility functions
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
/// \brief A string that describes the gnu version number, e.g., "1.0".
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
  /// \brief Retrieves the repository path (e.g., Subversion path) that
  /// identifies the particular Clang branch, tag, or trunk from which this
  /// Clang was built.
  std::string getClangRepositoryPath();

  /// \brief Retrieves the repository path from which LLVM was built.
  ///
  /// This supports LLVM residing in a separate repository from clang.
  std::string getLLVMRepositoryPath();

  /// \brief Retrieves the repository revision number (or identifer) from which
  /// this Clang was built.
  std::string getClangRevision();

  /// \brief Retrieves the repository revision number (or identifer) from which
  /// LLVM was built.
  ///
  /// If Clang and LLVM are in the same repository, this returns the same
  /// string as getClangRevision.
  std::string getLLVMRevision();

  /// \brief Retrieves the full repository version that is an amalgamation of
  /// the information in getClangRepositoryPath() and getClangRevision().
  std::string getClangFullRepositoryVersion();

  /// \brief Retrieves a string representing the complete clang version,
  /// which includes the clang version number, the repository version,
  /// and the vendor tag.
  std::string getClangFullVersion();

  /// \brief Like getClangFullVersion(), but with a custom tool name.
  std::string getClangToolFullVersion(llvm::StringRef ToolName);

  /// \brief Retrieves a string representing the complete clang version suitable
  /// for use in the CPP __VERSION__ macro, which includes the clang version
  /// number, the repository version, and the vendor tag.
  std::string getClangFullCPPVersion();

#if INTEL_CUSTOMIZATION
#ifdef INTEL_SPECIFIC_IL0_BACKEND
  /// \brief Retrieves a string representing the complete clang version suitable
  /// for use in the CPP __VERSION__ macro, which includes the clang version
  /// number, the repository version, and the vendor tag.
  ///
  /// Version string for iclang: cfe_iclangC/tr60450
  std::string getIClangFullCPPVersion();
#else // if !GNU_VERSION_STRING
  /// Version string for xmain: cq374831
  std::string getXMainFullCPPVersion();
#endif // INTEL_SPECIFIC_IL0_BACKEND
#endif // INTEL_CUSTOMIZATION
}

#endif // LLVM_CLANG_BASIC_VERSION_H
