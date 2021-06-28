//===------------------------ OptReportHandler.h ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Defines clang::OptReportHandler class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_OPTREPORTHANDLER_H
#define LLVM_CLANG_BASIC_OPTREPORTHANDLER_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"

// FIXME: This file needs to be refactored to fit better with its upstream
// version. The INTEL markers for the SYCL portions of this file are a result
// of naming inconsistencies between the two versions.

namespace clang {

class FunctionDecl;

class OptReportHandler { // INTEL
private:
  struct SyclOptReportInfo { // INTEL
    std::string KernelArgDescName; // Kernel argument name itself, or the name
                                   // of the parent class if the kernel argument
                                   // is a decomposed member.
    std::string KernelArgType;
    SourceLocation KernelArgLoc;
    unsigned KernelArgSize;
    std::string KernelArgDesc;
    std::string KernelArgDecomposedField;

    SyclOptReportInfo(std::string ArgDescName, std::string ArgType, // INTEL
                      SourceLocation ArgLoc, unsigned ArgSize,
                      std::string ArgDesc, std::string ArgDecomposedField)
        : KernelArgDescName(std::move(ArgDescName)),
          KernelArgType(std::move(ArgType)), KernelArgLoc(ArgLoc),
          KernelArgSize(ArgSize), KernelArgDesc(std::move(ArgDesc)),
          KernelArgDecomposedField(std::move(ArgDecomposedField)) {}
  };
  llvm::DenseMap<const FunctionDecl *, SmallVector<SyclOptReportInfo>>
      SyclMap; // INTEL

#if INTEL_CUSTOMIZATION
  struct OpenMPOptReportInfo {
    StringRef DirectiveKindName;
    SourceLocation DirectiveLoc;
    StringRef ClauseKindName;

    OpenMPOptReportInfo(StringRef DirName, SourceLocation Loc,
                        StringRef ClauseKind)
        : DirectiveKindName(DirName), DirectiveLoc(Loc),
          ClauseKindName(ClauseKind) {}
  };

  llvm::DenseMap<const FunctionDecl *, SmallVector<OpenMPOptReportInfo>>
      OpenMPMap;

public:
  void AddKernelArgs(const FunctionDecl *FD, StringRef ArgDescName,
                     StringRef ArgType, SourceLocation ArgLoc, unsigned ArgSize,
                     StringRef ArgDesc, StringRef ArgDecomposedField) {
    SyclMap[FD].emplace_back(ArgDescName.data(), ArgType.data(), ArgLoc,
                             ArgSize, ArgDesc.data(),
                             ArgDecomposedField.data());
  }
  SmallVector<SyclOptReportInfo> &GetSyclInfo(const FunctionDecl *FD) {
    auto It = SyclMap.find(FD);
    assert(It != SyclMap.end());
    return It->second;
  }

  bool HasSyclOptReportInfo(const FunctionDecl *FD) const {
    return SyclMap.find(FD) != SyclMap.end();
  }

  void AddIgnoredPragma(const FunctionDecl *FD, StringRef DirName,
                        SourceLocation Loc,
                        StringRef ClauseName = StringRef()) {
    OpenMPMap[FD].emplace_back(DirName, Loc, ClauseName);
  }
  SmallVector<OpenMPOptReportInfo> &GetClangInfo(const FunctionDecl *FD) {
    auto It = OpenMPMap.find(FD);
    assert(It != OpenMPMap.end());
    return It->second;
  }

  bool HasOpenMPReportInfo(const FunctionDecl *FD) {
    return OpenMPMap.find(FD) != OpenMPMap.end();
  }
#endif // INTEL_CUSTOMIZATION
};

} // namespace clang

#endif // LLVM_CLANG_BASIC_OPTREPORTHANDLER_H
