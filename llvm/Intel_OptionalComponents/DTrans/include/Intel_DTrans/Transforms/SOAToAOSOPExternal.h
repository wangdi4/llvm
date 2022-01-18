//===------------- SOAToAOSOPExternal.h - DTransSOAToAOSOPPass  -----------===//
//
// Copyright (C) 2021-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file exposes functionality needed for pre-LTO inlining heuristics.
// Most of the code (i.e SOAToAOSOPLayoutInfo and SOAToAOSOPCFGInfo) in this
// file is used in both SOAToAOS at LTO and Inliner at pre-LTO for
// inlining heuristics.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEXTERNAL_H
#define INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEXTERNAL_H

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "llvm/IR/Module.h"

// Same as in SOAToAOSOP.cpp.
#define DTRANS_LAYOUT_DEBUG_TYPE "dtrans-soatoaosop"

// Functionality needed by InlineCost.cpp.
namespace llvm {
namespace dtransOP {
namespace soatoaosOP {
// Utility to extract array type at offset Off from Struct,
// given Struct is a candidate for SOA-to-AOS.
inline DTransStructType *getOPSOAArrayType(DTransStructType *Struct,
                                           unsigned Off) {
  return cast<DTransStructType>(
      Struct->getFieldType(Off)->getPointerElementType());
}

// Utility to extract array type's element,
// given struct representing array.
inline DTransPointerType *getOPSOAElementType(DTransStructType *ArrayType,
                                              unsigned BasePointerOffset) {
  return cast<DTransPointerType>(
      ArrayType->getFieldType(BasePointerOffset)->getPointerElementType());
}

// Extract 'this` parameter and pointed-to type of 'this'.
inline DTransStructType *getOPStructTypeOfMethod(Function *F,
                                                 DTransSafetyInfo *DTInfo) {
  if (F->arg_size() < 1)
    return nullptr;

  DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
      DTInfo->getTypeMetadataReader().getDTransTypeFromMD(F));
  // Functions with no pointer types will be ignored.
  if (!DFnTy)
    return nullptr;

  if (auto *DPTy = dyn_cast<DTransPointerType>(DFnTy->getArgType(0)))
    if (auto *DSTy = dyn_cast<DTransStructType>(DPTy->getPointerElementType()))
      return DSTy;

  return nullptr;
}

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
  inline bool populateLayoutInformation(DTransType *DTy,
                                        DTransSafetyInfo *DTInfo);

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
  inline bool populateCFGInformation(Module &M, DTransSafetyInfo *DTInfo,
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

bool SOAToAOSOPLayoutInfo::populateLayoutInformation(DTransType *DTy,
                                                     DTransSafetyInfo *DTInfo) {

  // All struct types should pass this check.
  auto ExtractStructTy = [](DTransType *DTy) -> DTransStructType * {
    if (!DTy || !DTy->isStructTy())
      return nullptr;
    auto *St = cast<StructType>(DTy->getLLVMType());
    if (St->isLiteral() || !St->isSized())
      return nullptr;
    return cast<DTransStructType>(DTy);
  };

  // All pointers should pass this check.
  auto ExtractPointeeTy = [](DTransType *DTy) -> DTransType * {
    if (!DTy || !DTy->isPointerTy())
      return nullptr;
    auto *PTy = cast<PointerType>(DTy->getLLVMType());
    if (PTy->getAddressSpace())
      return nullptr;
    return DTy->getPointerElementType();
  };

  // Actually array of pointers to vararg functions.
  auto IsVTable = [&ExtractPointeeTy](DTransType *DFTy) -> bool {
    if (!DFTy)
      return false;
    if (auto *ElementTy = ExtractPointeeTy(ExtractPointeeTy(DFTy)))
      if (ElementTy->isFunctionTy())
        return true;

    return false;
  };

  // Empty class or interface (has only vtable).
  auto HasNoData = [&IsVTable](DTransStructType *DSTy) -> bool {
    if (DSTy->getNumFields() >= 2)
      return false;

    if (DSTy->getNumFields() == 1 && !IsVTable(DSTy->getFieldType(0)))
      return false;

    return true;
  };

  // Array of i8.
  // No access to these fields are allowed in
  // SOAToAOSOPArrays.h/SOAToAOSOPStruct.h
  auto IsPaddingFieldCandidate = [](DTransType *DTy) -> bool {
    if (!DTy)
      return false;
    auto *Ty = DTy->getLLVMType();
    if (!Ty->isArrayTy())
      return false;
    auto *El = Ty->getArrayElementType();
    if (!El->isIntegerTy() || El->getIntegerBitWidth() != 8)
      return false;
    return true;
  };

  auto StripTrivialDerivedClasses =
      [&IsPaddingFieldCandidate](DTransStructType *STy) -> DTransStructType * {
    auto *I = STy;
    for (; I && I->getNumFields() >= 1 && I->getNumFields() <= 2;
         I = dyn_cast_or_null<DTransStructType>(I->getFieldType(0))) {
      if (I->getNumFields() == 2)
        if (!IsPaddingFieldCandidate(I->getFieldType(1)))
          return I;
    }
    return I;
  };

  // Looks like array wrapper.
  auto IsPtrToArrayCandidate =
      [&ExtractStructTy, &ExtractPointeeTy, &IsVTable, &HasNoData,
       &IsPaddingFieldCandidate](DTransStructType *DSTy, bool &HasVT) -> bool {
    HasVT = false;
    unsigned NoDataPointerFields = 0;
    unsigned ElemDataPointerFields = 0;
    for (auto DEM : DSTy->elements()) {

      auto *DETy = DEM.getType();
      if (!DETy)
        return false;
      if (DETy->getLLVMType()->isIntegerTy())
        continue;

      if (IsVTable(DETy)) {
        HasVT = true;
        continue;
      }

      if (IsPaddingFieldCandidate(DETy))
        continue;

      auto *Pointee = ExtractPointeeTy(DETy);
      if (!Pointee)
        return false;

      if (auto St = ExtractStructTy(Pointee))
        if (HasNoData(St)) {
          ++NoDataPointerFields;
          continue;
        }

      // Only pointers are allowed as element.
      if (ExtractPointeeTy(Pointee)) {
        ++ElemDataPointerFields;
        continue;
      }

      return false;
    }

    if (NoDataPointerFields > 1 || ElemDataPointerFields != 1)
      return false;

    return true;
  };

  if (!DTInfo)
    return FALSE("no DTInfo");

  // Outer struct set.
  DStruct = ExtractStructTy(DTy);
  if (!DStruct)
    return FALSE("not valid struct.");
  auto *Struct = cast<StructType>(DStruct->getLLVMType());

  if (Struct->getNumElements() < 2)
    return FALSE("struct has 0 or 1 field.");

  // Set ArrayFieldOffsets and MemoryInterface.
  {
    unsigned NumberPtrsToNoData = 0;
    unsigned NumberPtrsToArr = 0;
    unsigned Offset = -1U;
    auto *DSTy = cast<DTransStructType>(DTy);
    for (auto &DFM : DSTy->elements()) {
      ++Offset;

      auto *DFTy = DFM.getType();
      if (!DFTy)
        return FALSE("field of struct has no type.");

      if (IsPaddingFieldCandidate(DFTy))
        continue;

      auto *Pointee = ExtractStructTy(ExtractPointeeTy(DFTy));
      if (!Pointee)
        return FALSE("struct has non-pointer field (ignoring padding).");

      if (HasNoData(Pointee)) {
        ++NumberPtrsToNoData;
        MemoryInterface = Pointee;
        continue;
      }

      if (auto *S = StripTrivialDerivedClasses(Pointee)) {
        bool HasVT = false;
        if (IsPtrToArrayCandidate(S, HasVT)) {
          ++NumberPtrsToArr;

          // Too many fields to analyze.
          if (MaxNumFieldTotalCandidates < NumberPtrsToArr)
            return FALSE("struct has too many pointers to arrays.");

          // Ignore classes with non-trivial base classes and/or vtable.
          if (!HasVT && S == Pointee)
            ArrayFieldOffsets.push_back(Offset);
          else
            PotentialArrayFieldOffsets.push_back(Offset);
          continue;
        }
      }

      return FALSE("struct has unsupported pointer fields.");
    }

    if (getNumArrays() < 2)
      return FALSE("struct has too few pointer to candidates.");

    if (getNumArrays() > MaxNumFieldCandidates)
      return FALSE("struct has too many pointers to arrays.");

    if (NumberPtrsToNoData > 1)
      return FALSE("struct has too many mem interface candidates.");
  }

  // Set BasePointerOffset.
  // Check first array candidate.
  {
    unsigned NumIntFields = 0;
    unsigned Offset = -1U;
    auto *S = *fields_begin();
    for (auto &DE : S->elements()) {
      ++Offset;

      auto *DETy = DE.getType();
      if (!DETy)
        return FALSE("field of struct has no type.");

      if (IsPaddingFieldCandidate(DETy))
        continue;

      auto *E = DETy->getLLVMType();
      if (E->isIntegerTy()) {
        ++NumIntFields;
        continue;
      }

      if (auto *F = ExtractStructTy(ExtractPointeeTy(DETy))) {
        if (MemoryInterface != F)
          return FALSE(
              "arrays and struct have different candidates for mem interface.");
        continue;
      }

      // Only pointers as elements are permitted.
      //  - This assumption simplifies allocation size computations.
      //  - Also it is likely that FunctionComparator only works with elements,
      //  which are layout-compatible.
      if (!ExtractPointeeTy(ExtractPointeeTy(DETy)))
        return FALSE("array has non-pointer element type.");

      BasePointerOffset = Offset;
    }

    // There should be at least 2 integer fields: 'capacity' and 'size' fields.
    // There should be base pointer field.
    if (NumIntFields > MaxNumIntFields)
      return FALSE("array has too many integer fields.");

    if (NumIntFields < 2)
      return FALSE("array has less than 2 integer fields.");

    if (BasePointerOffset == -1U)
      return FALSE("array has no base pointer candidate fields.");
  }

  // Compare classes representing arrays.
  {
    auto *S = *fields_begin();
    // Compare first array candidate with remaining.
    for (auto *S1 : fields()) {
      if (S1->getNumFields() != S->getNumFields())
        return FALSE("array structures have different number of fields.");

      for (auto Pair : zip_first(S->elements(), S1->elements())) {
        auto &EM = std::get<1>(Pair);
        auto *E = EM.getType();

        if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
          if (MemoryInterface != F)
            return FALSE("arrays and struct have different candidates for mem "
                         "interface.");
          continue;
        }

        auto &EM0 = std::get<0>(Pair);
        if (EM0.getType() == E)
          if (IsPaddingFieldCandidate(E) || E->getLLVMType()->isIntegerTy())
            continue;

        // Only pointers as elements are permitted.
        // This assumption simplifies allocation size computations.
        if (!ExtractPointeeTy(ExtractPointeeTy(E)))
          return FALSE("array has non-pointer element type.");

        assert(ExtractPointeeTy(ExtractPointeeTy(EM0.getType())) &&
               "Non-exhaustive checks of 0th candidate");
      }
    }
  }

  return true;
}

bool SOAToAOSOPCFGInfo::populateCFGInformation(Module &M,
                                               DTransSafetyInfo *DTInfo,
                                               bool RespectSizeHeuristic,
                                               bool RespectAttrs) {

  // Cannot process declarations and varargs.
  auto IsValidMethod = [RespectAttrs](Function &F) -> bool {
    if ((RespectAttrs && F.isDeclaration()) || F.isVarArg())
      return false;
    return true;
  };

  if (!DTInfo)
    return FALSE("No DTrans Info");

  // Initialize fields' methods to empty sets.
  ArrayFieldsMethods.assign(getNumArrays(), MethodSetTy());

  for (auto &F : M) {
    // Skip unused prototypes.
    if (F.isDeclaration() && F.use_empty())
      continue;

    if (auto *ThisTy = getOPStructTypeOfMethod(&F, DTInfo)) {
      if (ThisTy == DStruct) {

        if (!IsValidMethod(F))
          return FALSE("struct method has no definition or is vararg.");

        StructMethods.push_back(&F);
        continue;
      }

      // Tolerating linear search, because ArrayFieldOffsets.size() is very
      // small.
      for (auto Pair : zip_first(fields(), methodsets()))
        if (ThisTy == std::get<0>(Pair)) {
          if (!IsValidMethod(F))
            return FALSE("array method has no definition or is vararg.");

          // Structure of array should have simple call graph wrt to arrays'
          // methods.
          if (F.hasNUsesOrMore(MaxNumFieldMethodUses + 1) &&
              RespectSizeHeuristic)
            return FALSE("array method has too many calls to.");

          std::get<1>(Pair)->push_back(&F);
        }
    }
  }

  for (auto Triple : zip_first(methodsets(), elements(), fields())) {
    if (std::get<0>(Triple)->size() > MaxNumMethods)
      return FALSE("array has too many method to process.");

    for (auto *F : *std::get<0>(Triple)) {
      // Given that only primitive methods are recognized,
      // restrict the size.
      if (F->size() > MaxMethodBBCount && RespectSizeHeuristic)
        return FALSE("array method is too big.");

      DTransFunctionType *DFnTy = dyn_cast<DTransFunctionType>(
          DTInfo->getTypeMetadataReader().getDTransTypeFromMD(F));
      assert(DFnTy && "DTransType expected for function");

      unsigned NumArgs = F->arg_size();
      for (unsigned ArgIdx = 0; ArgIdx < NumArgs; ++ArgIdx) {
        DTransType *DTArgTy = DFnTy->getArgType(ArgIdx);
        assert(DTArgTy && "DTArgTy expected");

        auto *Ty = DTArgTy->getLLVMType();
        if (Ty->isIntegerTy())
          continue;
        // By-value argument of element type.
        if (DTArgTy == std::get<1>(Triple))
          continue;

        if (!DTArgTy->isPointerTy())
          return FALSE("array method has unexpected non-pointer parameter.");

        if (Ty->getPointerAddressSpace())
          return FALSE("array method has non-0 address space pointer.");

        auto *Pointee = DTArgTy->getPointerElementType();
        if (Pointee == MemoryInterface)
          continue;

        Argument *Arg = F->getArg(ArgIdx);
        for (unsigned I = 0, E = Attribute::EndAttrKinds; I != E; ++I) {
          auto Kind = static_cast<Attribute::AttrKind>(I);
          switch (Kind) {
            // only these are allowed:
          case Attribute::NonNull:
          case Attribute::NoCapture:
          case Attribute::ReadOnly:
          case Attribute::ReadNone:
          case Attribute::Dereferenceable:
          case Attribute::DereferenceableOrNull:
          case Attribute::Alignment:
          case Attribute::Returned:
          case Attribute::WriteOnly:
            continue;
          default:
            if (Arg->hasAttribute(Kind))
              return FALSE("array method has unexpected parameter attribute");
          }
        }

        // Only MemoryInterface and by-value argument of element type can be
        // captured.
        // Ctor/Dtor/CCtor have return types on windows and “this”
        // parameter is marked with “Returned”.
        if (RespectAttrs &&
            !(Arg->hasNoCaptureAttr() || Arg->hasReturnedAttr())) {
          return FALSE("array method captures unexpected parameter.");
        }

        // See list of methods supported.
        if (Pointee != std::get<1>(Triple) && Pointee != std::get<2>(Triple))
          return FALSE("array method has unsupported parameter.");
      }
    }
  }

  DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
    dbgs() << "  ; Struct's " << DStruct->getName() << " methods:\n";
    for (auto *F : StructMethods) {
      dbgs() << "   ; " << F->getName() << "\n";
    }

    for (auto Pair : zip_first(fields(), methodsets())) {
      dbgs() << "  ; Fields's " << std::get<0>(Pair)->getName()
             << " methods:\n";
      for (auto *F : *std::get<1>(Pair)) {
        dbgs() << "   ; " << F->getName() << ", #uses = " << F->getNumUses()
               << "\n";
      }
    }
  });

  return true;
}

} // namespace soatoaosOP
} // namespace dtransOP
} // namespace llvm
#endif // INTEL_DTRANS_TRANSFORMS_SOATOAOSOPEXTERNAL_H
