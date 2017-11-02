//===--- CGBuiltin_getBuiltinIntelLibFunction.cpp ---------------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
#include "CodeGenFunction.h"
#include "CGObjCRuntime.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"

using namespace clang;
using namespace CodeGen;

/// getBuiltinIntelLibFunction - Given a builtin id for a function like
/// "__apply_args", return a Function* for "__apply_args".
llvm::Constant *CodeGenModule::getBuiltinIntelLibFunction(
  const FunctionDecl *FD, unsigned BuiltinID) {
  // Get the name, skip over the __builtin_ prefix (if necessary).
  GlobalDecl D(FD);
  StringRef Name = Context.BuiltinInfo.getName(BuiltinID);

  llvm::FunctionType *Ty =
    cast<llvm::FunctionType>(getTypes().ConvertType(FD->getType()));

  return GetOrCreateLLVMFunction(Name, Ty, D, /*ForVTable=*/false);
}
