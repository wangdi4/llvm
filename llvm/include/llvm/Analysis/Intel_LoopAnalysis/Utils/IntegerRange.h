//===- IntegerRange.h - Helper class for iterating dimensions/loop levels -===//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Helper classes for iterating a integer range:
// Currently designed in a narrow scope to support following two cases.
//  1. RegDDRef's dimensions: dim_index_begin()/dim_index_end()
//             to iterate through 1 to getNumDimensions() inclusively.
//     ex)
//        for (auto I : make_range(Ref->dim_index_begin(),
//        Ref->dim_index_end()))
//          CanonExpr *CE = Ref->getNumDimension(I);
//
//  2. All possible loop levels, which is [1, MaxLoopNestLevel]. Refer to
//  IR/CanonExpr.h
//     ex) for (auto Level :
//                make_range(AllLoopLevel::begin(), AllLoopLevel::end()))
//     Notice it is the same as
//         for (unsigned I = 1; I <= MaxLoopNestLevel; I++)
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_INTEGERRANGE_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_UTILS_INTEGERRANGE_H

namespace llvm {
namespace loopopt {

class IntegerRangeIterator
    : public std::iterator<std::forward_iterator_tag, int, int, int *, int> {
private:
  // TODO: 32bit signed integer range is enough for our main use cases.
  //       Extend it when a need arises.
  int Val;

public:
  explicit IntegerRangeIterator(int num) : Val(num) {}
  explicit IntegerRangeIterator(unsigned num) : Val(num) {}

  IntegerRangeIterator &operator++() {
    Val++;
    return *this;
  }

  IntegerRangeIterator operator++(int) {
    IntegerRangeIterator RetVal = *this;
    ++(*this);
    return RetVal;
  }

  bool operator==(const IntegerRangeIterator &Other) const {
    return Val == Other.Val;
  }

  bool operator!=(const IntegerRangeIterator &Other) const {
    return !(*this == Other);
  }

  reference operator*() const { return Val; }
};

} // namespace loopopt
} // namespace llvm
#endif
