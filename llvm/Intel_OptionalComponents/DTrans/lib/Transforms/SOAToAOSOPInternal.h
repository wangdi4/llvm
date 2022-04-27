//===------------- SOAToAOSOPInternal.h - DTransSOAToAOSOPPass  -----------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains internal functionality needed for the SOAToAOS
// transformations.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOPINTERNAL_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOPINTERNAL_H

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/Module.h"

// Same as in SOAToAOSOP.cpp.
#define DTRANS_LAYOUT_DEBUG_TYPE "dtrans-soatoaosop"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

// Utility to extract array type at offset Off from Struct,
// given Struct is a candidate for SOA-to-AOS.
DTransStructType *getOPSOAArrayType(DTransStructType *Struct,
                                    unsigned Off);

// Utility to extract array type's element,
// given struct representing array.
DTransPointerType *getOPSOAElementType(DTransStructType *ArrayType,
                                       unsigned BasePointerOffset);

// Extract 'this` parameter and pointed-to type of 'this'.
DTransStructType *getOPStructTypeOfMethod(Function *F,
                                          TypeMetadataReader &MDReader);

// Extract 'this` parameter and pointed-to type of 'this'.
DTransStructType *getOPStructTypeOfMethod(Function *F,
                                          DTransSafetyInfo *DTInfo);

// Prepare layout information for candidates.
//
// %class.FieldValueMap = type {
//    %class.vector*,
//    %class.vector.0*,
//    %class.refvector*,
//    MemoryManager*
// }
//
// Makes sure pointer fields point to either vector element or
// MemoryManager.
// %class.vector = type {
//    i8, i32, i32,
//    %class.elem**,
//    MemoryManager*
// }
//
// Makes sure pointer fields point to either vector element or
// MemoryManager.
// %class.vector.0 = type {
//    i8, i32, i32,
//    %class.elem0**,
//    MemoryManager*
// }
class SOAToAOSOPLayoutInfo {
public:
  // Returns false if idiom is not recognized.
  // Single out-of-line method with lambdas.
  bool populateLayoutInformation(DTransType *DTy);

  constexpr static int MaxNumFieldTotalCandidates = 3;
  constexpr static int MaxNumFieldCandidates = 3;
  // There should be at least 'capacity' and 'size' fields.
  // Also permit auxiliary field.
  constexpr static int MaxNumIntFields = 3;

  using OffsetsTy = SmallVector<unsigned, MaxNumFieldCandidates>;

  // Helper class for fields* methods.
  template <typename IterTy>
  class ArrayIter
      : public iterator_adaptor_base<
            ArrayIter<IterTy>, IterTy,
            typename std::iterator_traits<IterTy>::iterator_category,
            DTransStructType *,
            typename std::iterator_traits<IterTy>::difference_type,
            DTransStructType **, DTransStructType *> {
    using BaseTy = iterator_adaptor_base<
        ArrayIter, IterTy,
        typename std::iterator_traits<IterTy>::iterator_category,
        DTransStructType *,
        typename std::iterator_traits<IterTy>::difference_type,
        DTransStructType **, DTransStructType *>;

  public:
    const SOAToAOSOPLayoutInfo *Info;
    ArrayIter(const SOAToAOSOPLayoutInfo *Info, IterTy It)
        : BaseTy(It), Info(Info) {}

    typename BaseTy::reference operator*() const {
      return getOPSOAArrayType(Info->DStruct, *this->wrapped());
    }
  };

  // Accessing structures representing arrays.
  using const_iterator = ArrayIter<OffsetsTy::const_iterator>;

  const_iterator fields_begin() const {
    return const_iterator(this, ArrayFieldOffsets.begin());
  }
  const_iterator fields_end() const {
    return const_iterator(this, ArrayFieldOffsets.end());
  }
  iterator_range<const_iterator> fields() const {
    return make_range(fields_begin(), fields_end());
  }

  // Returns number of potential array fields.
  unsigned getNumPotentialArrays() const {
    return PotentialArrayFieldOffsets.size();
  }
  // Iterator for PotentialArrayFieldOffsets.
  typedef OffsetsTy::const_iterator p_f_const_iterator;
  inline iterator_range<p_f_const_iterator> potential_arr_fields() {
    return make_range(PotentialArrayFieldOffsets.begin(),
                      PotentialArrayFieldOffsets.end());
  }

protected:
  // Helper class for elements* methods.
  template <typename IterTy>
  class ElementIter
      : public iterator_adaptor_base<
            ElementIter<IterTy>, IterTy,
            typename std::iterator_traits<IterTy>::iterator_category,
            DTransPointerType *,
            typename std::iterator_traits<IterTy>::difference_type,
            DTransPointerType **, DTransPointerType *> {
    using BaseTy = iterator_adaptor_base<
        ElementIter, OffsetsTy::const_iterator,
        typename std::iterator_traits<IterTy>::iterator_category,
        DTransPointerType *,
        typename std::iterator_traits<IterTy>::difference_type,
        DTransPointerType **, DTransPointerType *>;

  public:
    const SOAToAOSOPLayoutInfo *Info;
    ElementIter(const SOAToAOSOPLayoutInfo *Info, IterTy It)
        : BaseTy(It), Info(Info) {}

    typename BaseTy::reference operator*() const {
      return getOPSOAElementType(
          getOPSOAArrayType(Info->DStruct, *this->wrapped()),
          Info->BasePointerOffset);
    }
  };

  // Accessing types of elements stored in arrays.
  using const_elem_iterator = ElementIter<OffsetsTy::const_iterator>;

  const_elem_iterator elements_begin() const {
    return const_elem_iterator(this, ArrayFieldOffsets.begin());
  }
  const_elem_iterator elements_end() const {
    return const_elem_iterator(this, ArrayFieldOffsets.end());
  }
  iterator_range<const_elem_iterator> elements() const {
    return make_range(elements_begin(), elements_end());
  }

  // Hook point. Top-level returns from populate* methods.
  static bool FALSE(const char *Msg) {
    DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE,
                    dbgs() << "; dtrans-soatoaos-layout: " << Msg << "\n");
    return false;
  }

  unsigned getNumArrays() const { return ArrayFieldOffsets.size(); }

  DTransStructType *DStruct = nullptr;
  // Memory management pointer.
  DTransStructType *MemoryInterface = nullptr;
  // Offsets in Struct's elements() to represent pointers to candidate
  // _arrays_. _Arrays_ are represented as some classes.
  OffsetsTy ArrayFieldOffsets;

  // Offsets of Struct's elements that are not selected by SOAToAOSOP
  // transformation as candidates due to layout issues but they are
  // potential candidates for the transformation.
  OffsetsTy PotentialArrayFieldOffsets;

  // Offset inside _arrays_' elements(), which represent base pointers to
  // allocated memory.
  unsigned BasePointerOffset = -1U;
};

// Prepare CFG information and methods to analyze.
// Cheap checks are performed.
class SOAToAOSOPCFGInfo : public SOAToAOSOPLayoutInfo {
public:
  // Returns false if idiom is not recognized.
  // Single out-of-line method with lambdas.
  //
  // Checks that
  // 1. arrays' methods are small and have at most MaxNumFieldMethodUses call
  //    sites;
  // 2. arguments of arrays' methods are 'this' pointers (not captured),
  //    elements of array (not captured if passed by reference),
  //    integers and MemoryInterface.
  //
  // Suitable for heuristic.
  bool populateCFGInformation(Module &M,
                              TypeMetadataReader &MDReader,
                              bool RespectSizeHeuristic,
                              bool RespectAttrs);

  constexpr static int NumRequiredMethods = 4;
  // Specific methods (3) and integer fields accessors (3).
  constexpr static int NumSupportedMethods = 3 + 3;
  constexpr static int MaxNumMethods = NumRequiredMethods + NumSupportedMethods;
  constexpr static int MaxMethodBBCount = 24;

  // Field methods are used only in Struct's methods,
  // it should not be too big.
  constexpr static int MaxNumFieldMethodUses = 2;

  void collectFuncs(SmallSet<Function *, 20> *FuncsForDtrans) const {
    for (auto *F : StructMethods)
      FuncsForDtrans->insert(F);
    for (auto *S : methodsets())
      for (auto *F : *S)
        FuncsForDtrans->insert(F);
  }

protected:
  using MethodSetTy = SmallVector<Function *, MaxNumMethods>;
  using ArrayMethodSetTy = SmallVector<MethodSetTy, MaxNumFieldCandidates>;

  // Helper class for methodsets* methods.
  template <typename IterTy, typename MethSetTy>
  class MethodsIter
      : public iterator_adaptor_base<
            MethodsIter<IterTy, MethSetTy>, IterTy,
            typename std::iterator_traits<IterTy>::iterator_category,
            MethSetTy *, typename std::iterator_traits<IterTy>::difference_type,
            MethSetTy **, MethSetTy *> {
    using BaseTy = iterator_adaptor_base<
        MethodsIter, IterTy,
        typename std::iterator_traits<IterTy>::iterator_category, MethSetTy *,
        typename std::iterator_traits<IterTy>::difference_type, MethSetTy **,
        MethSetTy *>;

  public:
    MethodsIter(IterTy It) : BaseTy(It) {}

    typename BaseTy::reference operator*() const { return &*this->wrapped(); }
  };

  // Updating method sets of arrays.
  using iterator = MethodsIter<ArrayMethodSetTy::iterator, MethodSetTy>;

  iterator methodsets_begin() { return iterator(ArrayFieldsMethods.begin()); }
  iterator methodsets_end() { return iterator(ArrayFieldsMethods.end()); }
  iterator_range<iterator> methodsets() {
    return make_range(methodsets_begin(), methodsets_end());
  }

  using const_iterator =
      MethodsIter<ArrayMethodSetTy::const_iterator, const MethodSetTy>;
  const_iterator methodsets_begin() const {
    return const_iterator(ArrayFieldsMethods.begin());
  }
  const_iterator methodsets_end() const {
    return const_iterator(ArrayFieldsMethods.end());
  }
  iterator_range<const_iterator> methodsets() const {
    return make_range(methodsets_begin(), methodsets_end());
  }

  MethodSetTy StructMethods;
  // It is matched by ArrayFieldsMethods,
  // can be iterated in parallel using zip_first.
  ArrayMethodSetTy ArrayFieldsMethods;
};

} // namespace soatoaosOP
} // namespace dtransOP
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOPINTERNAL_H
