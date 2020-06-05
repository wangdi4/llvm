//===--- HIRStencilPattern.cpp - Impl of Utils for checking stencil   ---===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
#include "HIRStencilPattern.h"
namespace llvm {

namespace loopopt {

#define LLVM_DEBUG_STENCIL(X) DEBUG_WITH_TYPE("hir-stencil-pattern", X)

namespace stencilpattern {

bool areStructuallyStencilRefs(RefGroupTy &Group) {

  const RegDDRef *Median = getMedianRef(Group);
  LLVM_DEBUG_STENCIL(dbgs() << "Median :"; Median->dump(); dbgs() << " ";);

  return isSymetricCenteredAt(Median, Group);
}

const RegDDRef *getMedianRef(RefGroupTy &Group) {
  std::nth_element(Group.begin(), Group.begin() + Group.size() / 2, Group.end(),
                   DDRefUtils::compareMemRefAddress);
  return Group[Group.size() / 2];
}

bool isSymetricCenteredAt(const RegDDRef *Center, const RefGroupTy &Group) {
  unsigned numCEs = Center->getNumDimensions();

  for (auto *Ref : Group) {
    if (Ref->getNumDimensions() != numCEs)
      return false;

    unsigned numNonZeroDist = 0;
    for (auto DimNum :
         make_range(Ref->dim_num_begin(), Ref->dim_num_end())) {
      int64_t Dist = 0;
      if (!CanonExprUtils::getConstDistance(Center->getDimensionIndex(DimNum),
                                            Ref->getDimensionIndex(DimNum),
                                            &Dist))
        return false;

      if (Dist != 0)
        numNonZeroDist++;
    }

    // At leas one dimension has const dist of zero.
    // TODO: This solves rhs_body, but might need more checks to be
    //       conservative enough as stencil function
    if (numNonZeroDist >= numCEs) {
      LLVM_DEBUG_STENCIL(dbgs() << "Problem Ref:"; Ref->dump(); dbgs() << "\n";
                         dbgs() << "F 4: group size: " << Group.size() << "\n";
                         for (auto *Ref
                              : Group) {
                           Ref->dump();
                           dbgs() << "\n";
                         });
      return false;
    }
  }

  return true;
}

} // namespace stencilpattern

} // namespace loopopt

} // namespace llvm
