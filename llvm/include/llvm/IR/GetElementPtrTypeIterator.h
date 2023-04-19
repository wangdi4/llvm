//===- GetElementPtrTypeIterator.h ------------------------------*- C++ -*-===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements an iterator for walking through the types indexed by
// getelementptr instructions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_GETELEMENTPTRTYPEITERATOR_H
#define LLVM_IR_GETELEMENTPTRTYPEITERATOR_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/User.h"
#include "llvm/Support/Casting.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>

#if INTEL_CUSTOMIZATION
#include <type_traits>
#endif // INTEL_CUSTOMIZATION

namespace llvm {

#if INTEL_CUSTOMIZATION
// Traits that get specialized by the user to specify how to obtain an LLVM
// Constant from a given value. This is used by generic_gep_type_iterator to
// obtain a Constant from the current operand.
template <typename T> struct GEPTypeIterTraits {
  static Constant *getConstant(T *);
};
template <>
inline Constant *GEPTypeIterTraits<llvm::Value>::getConstant(llvm::Value *V) {
  return dyn_cast<Constant>(V);
}
#endif // INTEL_CUSTOMIZATION

template <typename ItTy = User::const_op_iterator>
class generic_gep_type_iterator {

  ItTy OpIt;
  PointerUnion<StructType *, Type *> CurTy;
#if INTEL_CUSTOMIZATION
  enum : uint64_t { Unbounded = -1ull };
  uint64_t NumElements = Unbounded;
#endif // INTEL_CUSTOMIZATION

  generic_gep_type_iterator() = default;

#if INTEL_CUSTOMIZATION
  // The (non-const) type of values obtained by dereferencing the inner
  // iterator. In the LLVM case, this is Value, though it depends on the
  // underlying IR the iterator was obtained from; for example, in VPlan-IR,
  // this is VPValue.
  using NonConstValTy =
      std::remove_const_t<std::remove_reference_t<decltype(**OpIt)>>;

  // Get the constant corresponding to the current operand. This is specialized
  // according to the underlying IR, as different implementations must specify
  // how to extract a Constant from the current operand.
  Constant *getConstantOperand() const {
    return llvm::GEPTypeIterTraits<NonConstValTy>::getConstant(getOperand());
  }
#endif // INTEL_CUSTOMIZATION

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = Type *;
  using difference_type = std::ptrdiff_t;
  using pointer = value_type *;
  using reference = value_type &;

  static generic_gep_type_iterator begin(Type *Ty, ItTy It) {
    generic_gep_type_iterator I;
    I.CurTy = Ty;
    I.OpIt = It;
    return I;
  }

  static generic_gep_type_iterator end(ItTy It) {
    generic_gep_type_iterator I;
    I.OpIt = It;
    return I;
  }

  bool operator==(const generic_gep_type_iterator &x) const {
    return OpIt == x.OpIt;
  }

  bool operator!=(const generic_gep_type_iterator &x) const {
    return !operator==(x);
  }

  // FIXME: Make this the iterator's operator*() after the 4.0 release.
  // operator*() had a different meaning in earlier releases, so we're
  // temporarily not giving this iterator an operator*() to avoid a subtle
  // semantics break.
  Type *getIndexedType() const {
    if (auto *T = dyn_cast_if_present<Type *>(CurTy))
      return T;
#if INTEL_CUSTOMIZATION
    return cast<StructType *>(CurTy)->getTypeAtIndex(getConstantOperand());
  }

  NonConstValTy *getOperand() const {
    return const_cast<NonConstValTy *>(&**OpIt);
  }
#endif // INTEL_CUSTOMIZATION

  generic_gep_type_iterator &operator++() { // Preincrement
    Type *Ty = getIndexedType();
#if INTEL_CUSTOMIZATION
    if (auto *ATy = dyn_cast<ArrayType>(Ty)) {
      CurTy = ATy->getElementType();
      NumElements = ATy->getNumElements();
    } else if (auto *VTy = dyn_cast<VectorType>(Ty)) {
      CurTy = VTy->getElementType();
      if (isa<ScalableVectorType>(VTy))
        NumElements = Unbounded;
      else
        NumElements = cast<FixedVectorType>(VTy)->getNumElements();
    } else
#endif // INTEL_CUSTOMIZATION
      CurTy = dyn_cast<StructType>(Ty);
    ++OpIt;
    return *this;
  }

  generic_gep_type_iterator operator++(int) { // Postincrement
    generic_gep_type_iterator tmp = *this;
    ++*this;
    return tmp;
  }

  // All of the below API is for querying properties of the "outer type", i.e.
  // the type that contains the indexed type. Most of the time this is just
  // the type that was visited immediately prior to the indexed type, but for
  // the first element this is an unbounded array of the GEP's source element
  // type, for which there is no clearly corresponding IR type (we've
  // historically used a pointer type as the outer type in this case, but
  // pointers will soon lose their element type).
  //
  // FIXME: Most current users of this class are just interested in byte
  // offsets (a few need to know whether the outer type is a struct because
  // they are trying to replace a constant with a variable, which is only
  // legal for arrays, e.g. canReplaceOperandWithVariable in SimplifyCFG.cpp);
  // we should provide a more minimal API here that exposes not much more than
  // that.

  bool isStruct() const { return isa<StructType *>(CurTy); }
  bool isSequential() const { return isa<Type *>(CurTy); }

  StructType *getStructType() const { return cast<StructType *>(CurTy); }

  StructType *getStructTypeOrNull() const {
    return dyn_cast_if_present<StructType *>(CurTy);
  }

#if INTEL_CUSTOMIZATION
  bool isBoundedSequential() const {
    return isSequential() && NumElements != Unbounded;
  }
#endif // INTEL_CUSTOMIZATION
};

  using gep_type_iterator = generic_gep_type_iterator<>;

#if INTEL_CUSTOMIZATION
  /// Generic method to get a GEP begin iterator for a given user type. This can
  /// be called directly, but users should generally prefer the non-templated
  /// `gep_type_begin` interface.
  template <typename UserTy, typename ItTy, typename GEPTy>
  auto generic_gep_type_begin(const UserTy *U) -> ItTy {
    auto *GEP = cast<GEPTy>(U);
    return ItTy::begin(GEP->getSourceElementType(), U->op_begin() + 1);
  }

  /// Generic method to get a GEP end iterator for a given user type. This can
  /// be called directly, but users should generally prefer the non-templated
  /// `gep_type_end` interface.
  template <typename UserTy, typename ItTy>
  auto generic_gep_type_end(const UserTy *U) -> ItTy {
    return ItTy::end(U->op_end());
  }

  inline gep_type_iterator gep_type_begin(const User *U) {
    return generic_gep_type_begin<User, gep_type_iterator, GEPOperator>(U);
  }
  inline gep_type_iterator gep_type_end(const User *U) {
    return generic_gep_type_end<User, gep_type_iterator>(U);
  }
  inline gep_type_iterator gep_type_begin(const User &U) {
    return generic_gep_type_begin<User, gep_type_iterator, GEPOperator>(&U);
  }
  inline gep_type_iterator gep_type_end(const User &U) {
    return generic_gep_type_end<User, gep_type_iterator>(&U);
  }
#endif // INTEL_CUSTOMIZATION

  template<typename T>
  inline generic_gep_type_iterator<const T *>
  gep_type_begin(Type *Op0, ArrayRef<T> A) {
    return generic_gep_type_iterator<const T *>::begin(Op0, A.begin());
  }

  template<typename T>
  inline generic_gep_type_iterator<const T *>
  gep_type_end(Type * /*Op0*/, ArrayRef<T> A) {
    return generic_gep_type_iterator<const T *>::end(A.end());
  }

} // end namespace llvm

#endif // LLVM_IR_GETELEMENTPTRTYPEITERATOR_H
