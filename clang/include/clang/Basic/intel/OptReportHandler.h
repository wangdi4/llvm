//==--- OptReportHandler.h - Capture OpenMP Opt Report Info ---*- C++ -*---==//
//
// Copyright (C) 2019-2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_BASIC_OPTREPORTHANDLER_H
#define LLVM_CLANG_BASIC_OPTREPORTHANDLER_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"

namespace clang {

class FunctionDecl;

/// ClangOptReportHandler - Provides a mechanism to store information needed
/// in the optimization report.  The opt report calls cannot be made until
/// codegen but information on ignored pragmas must be generated during parsing.



class ClangOptReportHandler {
private:
  struct OptReportInfo {
    StringRef DirectiveKindName;
    SourceLocation DirectiveLoc;
    StringRef ClauseKindName;

    OptReportInfo(StringRef DirName, SourceLocation Loc,
                  StringRef ClauseKind)
        : DirectiveKindName(DirName), DirectiveLoc(Loc),
          ClauseKindName(ClauseKind) {}
  };
  llvm::DenseMap<const FunctionDecl *, SmallVector<OptReportInfo, 4>> Map;

public:
  void AddIgnoredPragma(const FunctionDecl *FD, StringRef DirName,
                        SourceLocation Loc,
                        StringRef ClauseName = StringRef()) {
    Map[FD].emplace_back(DirName, Loc, ClauseName);
  }

  bool HasOptReportInfo(const FunctionDecl *FD) {
    return Map.find(FD) != Map.end();
  }

  SmallVector<OptReportInfo, 4> &getInfo(const FunctionDecl *FD) {
    auto It = Map.find(FD);
    assert(It != Map.end());
    return It->second;
  }
};

} // namespace clang
#endif
