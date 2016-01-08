//===-- DDRefGrouping.h - Implements DDRef Grouping utilities -*-- C++ --*-===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// DDRefGrouping is an utility class to create DDRef groups
//
// A reference group is created at the specified level. All references
// in the group are differ by a constant.
//
// Simple Example:
// MemRefMap : B[J][I], B[J][I-1], B[J][I+1], B[J-1][I-1], B[J+1][I]
// RefGroup created for I-Level:
// Group 1 : B[J][I], B[J][I-1], B[J][I+1]
// Group 2 : B[J-1][I]
// Group 3 : B[J+1][I]
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGROUPING_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_DDREFGROUPING_H

#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefGatherer.h"

namespace llvm {

namespace loopopt {

class DDRefGrouping {
public:
  typedef MemRefGatherer::MapTy SymToMemRefTy;

  typedef SmallVector<const RegDDRef *, 32> RefGroupTy;

  // RefGroupsTy data structure.
  // The first unsigned argument is the group number.
  typedef std::map<unsigned, RefGroupTy> RefGroupsTy;

private:
  DDRefGrouping() = delete;
  ~DDRefGrouping() = delete;
  DDRefGrouping(const DDRefGathererUtils &) = delete;
  DDRefGrouping &operator=(const DDRefGrouping &) = delete;

  /// \brief Returns true if Ref2 belongs to the same array reference group.
  static bool isGroupMemRefMatch(const RegDDRef *Ref1, const RegDDRef *Ref2,
                          unsigned Level, uint64_t MaxDiff);

public:
  /// \brief Creates a reference group out of the Symbol to Mem Ref Table.
  static void createGroups(RefGroupsTy &Groups, const SymToMemRefTy &MemRefMap,
                    unsigned Level, uint64_t MaxDiff);

  /// \brief Prints out the array reference group mapping.
  /// Primarily used for debugging.
  static void dump(const RefGroupsTy &Groups);
};

}

}

#endif
