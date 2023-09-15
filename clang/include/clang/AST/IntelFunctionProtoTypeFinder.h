//===--- IntelFunctionProtoTypeFinder.h - Find FunctionProtoTypes -*- C++ -*-==///
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_AST_FUNCTIONPROTOTYPEFINDER_H
#define LLVM_CLANG_AST_FUNCTIONPROTOTYPEFINDER_H

#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/SmallVector.h"

namespace clang {

/// Find the list of FunctionProtoTypes within the Type.
class FunctionProtoTypeFinder final
    : public RecursiveASTVisitor<FunctionProtoTypeFinder> {
  llvm::SmallVector<const FunctionProtoType *, 4> Types;

public:
  bool TraverseFunctionProtoType(const FunctionProtoType *T) {
    Types.push_back(T);
    return true;
  }
  const SmallVectorImpl<const FunctionProtoType *> &
  getProtoTypes(const QualType T) {
    TraverseType(T);
    return Types;
  }
};

} // end namespace clang

#endif // LLVM_CLANG_AST_FUNCTIONPROTOTYPEFINDER_H
