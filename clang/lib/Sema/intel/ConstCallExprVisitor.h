//==--- ConstCallExprVisitor.h -                               -*- C++ -*---==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_CLANG_LIB_SEMA_INTEL_CONSTCALLEXPRVISITOR_H
#define LLVM_CLANG_LIB_SEMA_INTEL_CONSTCALLEXPRVISITOR_H

#include "clang/AST/StmtVisitor.h"
#include "llvm/ADT/SmallVector.h"

namespace {

using namespace clang;

/// Recursive AST Visitor to find all calls in the specified FunctionDecl
class ConstCallExprVisitor : public ConstStmtVisitor<ConstCallExprVisitor> {
public:
  void VisitStmt(const Stmt *S) { VisitChildren(S); }

  void VisitChildren(const Stmt *S) {
    for (const Stmt *Child : S->children()) {
      if (Child)
        this->Visit(Child);
    }
  }

  void VisitCallExpr(const CallExpr *CE) {
    if (!CE->getDirectCallee())
      return;

    Calls.push_back(CE);
  }

  void TraverseFunctionDecl(const FunctionDecl *FD) {
    if (!FD->hasBody())
      return;
    VisitChildren(FD->getBody());
  }

  llvm::SmallVectorImpl<const CallExpr *> &GetCalls() { return Calls; }

private:
  llvm::SmallVector<const CallExpr *, 16> Calls;
};

} // namespace

#endif // LLVM_CLANG_LIB_SEMA_INTEL_CONSTCALLEXPRVISITOR_H
