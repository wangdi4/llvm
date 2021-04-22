//==--- IntelAST.cpp           ---------------------------------*- C++ -*---==//
//
// Copyright (C) Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "clang/AST/ASTContext.h"
#include "llvm/Support/Intel_CPU_utils.h"
#include <vector>

using namespace clang;

template <typename F>
static void getCpuFeaturesFromBitmaskHelper(F AddFunc, uint64_t Page1,
                                            uint64_t Page2) {
  unsigned BitPos = 0;

  while (Page1 != 0) {
    if (Page1 & 1)
      AddFunc(llvm::X86::getCpuFeatureFromBitPosition(BitPos));
    ++BitPos;
    Page1 >>= 1;
  }

  BitPos = 64;

  while (Page2 != 0) {
    if (Page2 & 1)
      AddFunc(llvm::X86::getCpuFeatureFromBitPosition(BitPos));
    ++BitPos;
    Page2 >>= 1;
  }
}

void ASTContext::getAddCpuFeaturesFromBitmask(
    std::vector<std::string> &Features, uint64_t Page1, uint64_t Page2) const {
  getCpuFeaturesFromBitmaskHelper(
      [&Features](StringRef R) { Features.push_back('+' + R.str()); }, Page1,
      Page2);
}

void ASTContext::getCpuFeaturesFromBitmask(
    llvm::SmallVectorImpl<StringRef> &Features, uint64_t Page1,
    uint64_t Page2) const {
  getCpuFeaturesFromBitmaskHelper(
      [&Features](StringRef R) { Features.push_back(R); }, Page1, Page2);
}

bool ASTContext::isValidCpuFeaturesBitmask(unsigned Page, uint64_t Mask) const {
  unsigned BitPos = 0;
  while (Mask != 0) {
    if (Mask & 1)
      if (llvm::X86::getCpuFeatureFromBitPosition(BitPos + Page * 64) == "")
        return false;
    ++BitPos;
    Mask >>= 1;
  }
  return true;
}
