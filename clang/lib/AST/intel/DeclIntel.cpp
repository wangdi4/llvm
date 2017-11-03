//===--- DeclIntel.cpp - Intel Decl AST Node Implementation -----*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl subclasses.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTLambda.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Builtins.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "llvm/Support/ErrorHandling.h"
#include <algorithm>

using namespace clang;

#if INTEL_SPECIFIC_CILKPLUS

CilkSpawnDecl::CilkSpawnDecl(DeclContext *DC, CapturedStmt *Spawn) :
  Decl(CilkSpawn, DC, Spawn->getLocStart()), CapturedSpawn(Spawn) {
}

CilkSpawnDecl *CilkSpawnDecl::Create(ASTContext &C, DeclContext *DC,
                                     CapturedStmt *Spawn) {
  return new (C, DC) CilkSpawnDecl(DC, Spawn);
}

CilkSpawnDecl *CilkSpawnDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) CilkSpawnDecl(nullptr, nullptr);
}

Stmt *CilkSpawnDecl::getSpawnStmt() {
  return getCapturedStmt()->getCapturedStmt();
}

bool CilkSpawnDecl::hasReceiver() const {
  const Stmt *S = getSpawnStmt();
  assert(S && "null spawn statement");
  return isa<DeclStmt>(S);
}

VarDecl *CilkSpawnDecl::getReceiverDecl() const {
  Stmt *S = const_cast<Stmt *>(getSpawnStmt());
  assert(S && "null spawn statement");
  if (DeclStmt *DS = dyn_cast<DeclStmt>(S)) {
    assert(DS->isSingleDecl() && "single declaration expected");
    return cast<VarDecl>(DS->getSingleDecl());
  }

  return 0;
}
#endif // INTEL_SPECIFIC_CILKPLUS

#ifdef INTEL_SPECIFIC_IL0_BACKEND
void PragmaDecl::anchor() { }

PragmaDecl *PragmaDecl::Create(ASTContext &C, DeclContext *DC,
                         SourceLocation IdentL) {
  return new (C, DC) PragmaDecl(DC, IdentL);
}

PragmaDecl *PragmaDecl::CreateDeserialized(ASTContext &C, unsigned ID) {
  return new (C, ID) PragmaDecl(nullptr, SourceLocation());
}
#endif // INTEL_SPECIFIC_IL0_BACKEND
