//===--------------- SOAToAOSInternal.cpp - DTransSOAToAOSPass  -----------===//
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

#include "SOAToAOSInternal.h"

namespace llvm {
namespace dtrans {
namespace soatoaos {

StructType *getSOAArrayType(StructType *Struct, unsigned Off) {
  return cast<StructType>(Struct->getElementType(Off)->getPointerElementType());
}

PointerType *getSOAElementType(StructType *ArrayType,
                               unsigned BasePointerOffset) {
  return cast<PointerType>(
      ArrayType->getElementType(BasePointerOffset)->getPointerElementType());
}

StructType *getStructTypeOfMethod(const Function &F) {
  FunctionType *FTy = F.getFunctionType();
  if (FTy->getNumParams() < 1)
    return nullptr;

  if (auto *PTy = dyn_cast<PointerType>(FTy->getParamType(0)))
    if (auto *STy = dyn_cast<StructType>(PTy->getPointerElementType()))
      return STy;

  return nullptr;
}

bool SOAToAOSLayoutInfo::populateLayoutInformation(Type *Ty) {

  // All struct types should pass this check.
  auto ExtractStructTy = [](Type *Ty) -> StructType * {
    if (auto *St = dyn_cast_or_null<StructType>(Ty))
      if (!St->isLiteral() && St->isSized())
        return St;
    return nullptr;
  };

  // All pointers should pass this check.
  auto ExtractPointeeTy = [](Type *Ty) -> Type * {
    if (auto *PTy = dyn_cast_or_null<PointerType>(Ty)) {
      if (PTy->getAddressSpace())
        return nullptr;
      return PTy->getElementType();
    }
    return nullptr;
  };

  // Actually array of pointers to vararg functions.
  auto IsVTable = [&ExtractPointeeTy](Type *FTy) -> bool {
    if (auto *ElementTy = ExtractPointeeTy(ExtractPointeeTy(FTy)))
      if (ElementTy->isFunctionTy())
        return true;

    return false;
  };

  // Empty class or interface (has only vtable).
  auto HasNoData = [&IsVTable](StructType *STy) -> bool {
    if (STy->getNumElements() >= 2)
      return false;

    if (STy->getNumElements() == 1 && !IsVTable(STy->getElementType(0)))
      return false;

    return true;
  };

  // Array of i8.
  // No access to these fields are allowed in SOAToAOSArrays.h/SOAToAOSStruct.h
  auto IsPaddingFieldCandidate = [](Type *Ty) -> bool {
    if (!Ty->isArrayTy())
      return false;
    auto *El = Ty->getArrayElementType();
    if (!El->isIntegerTy() || El->getIntegerBitWidth() != 8)
      return false;
    return true;
  };

  auto StripTrivialDerivedClasses =
      [&IsPaddingFieldCandidate](StructType *STy) -> StructType * {
    auto *I = STy;
    for (; I && I->getNumElements() >= 1 && I->getNumElements() <= 2;
         I = dyn_cast<StructType>(I->getElementType(0))) {
      if (I->getNumElements() == 2)
        if (!IsPaddingFieldCandidate(I->getElementType(1)))
          return I;
    }
    return I;
  };

  // Looks like array wrapper.
  auto IsPtrToArrayCandidate =
      [&ExtractStructTy, &ExtractPointeeTy, &IsVTable, &HasNoData,
       &IsPaddingFieldCandidate](StructType *STy, bool &HasVT) -> bool {
    HasVT = false;
    unsigned NoDataPointerFields = 0;
    unsigned ElemDataPointerFields = 0;
    for (auto *ETy : STy->elements()) {
      if (ETy->isIntegerTy())
        continue;

      if (IsVTable(ETy)) {
        HasVT = true;
        continue;
      }

      if (IsPaddingFieldCandidate(ETy))
        continue;

      auto *Pointee = ExtractPointeeTy(ETy);
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

  // Outer struct set.
  Struct = ExtractStructTy(Ty);

  if (!Struct)
    return FALSE("not struct.");

  if (Struct->getNumElements() < 2)
    return FALSE("struct has 0 or 1 field.");

  // Set ArrayFieldOffsets and MemoryInterface.
  {
    unsigned NumberPtrsToNoData = 0;
    unsigned NumberPtrsToArr = 0;
    unsigned Offset = -1U;
    for (auto *FTy : Struct->elements()) {
      ++Offset;

      if (IsPaddingFieldCandidate(FTy))
        continue;

      auto *Pointee = ExtractStructTy(ExtractPointeeTy(FTy));
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
    for (auto E : S->elements()) {
      ++Offset;

      if (IsPaddingFieldCandidate(E))
        continue;

      if (E->isIntegerTy()) {
        ++NumIntFields;
        continue;
      }

      if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
        if (MemoryInterface != F)
          return FALSE(
              "arrays and struct have different candidates for mem interface.");
        continue;
      }

      // Only pointers as elements are permitted.
      //  - This assumption simplifies allocation size computations.
      //  - Also it is likely that FunctionComparator only works with elements,
      //  which are layout-compatible.
      if (!ExtractPointeeTy(ExtractPointeeTy(E)))
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
      if (S1->getNumElements() != S->getNumElements())
        return FALSE("array structures have different number of fields.");

      for (auto Pair : zip_first(S->elements(), S1->elements())) {
        auto *E = std::get<1>(Pair);

        if (auto *F = ExtractStructTy(ExtractPointeeTy(E))) {
          if (MemoryInterface != F)
            return FALSE("arrays and struct have different candidates for mem "
                         "interface.");
          continue;
        }

        if (std::get<0>(Pair) == E)
          if (IsPaddingFieldCandidate(E) || E->isIntegerTy())
            continue;

        // Only pointers as elements are permitted.
        // This assumption simplifies allocation size computations.
        if (!ExtractPointeeTy(ExtractPointeeTy(E)))
          return FALSE("array has non-pointer element type.");

        assert(ExtractPointeeTy(ExtractPointeeTy(std::get<0>(Pair))) &&
               "Non-exhaustive checks of 0th candidate");
      }
    }
  }

  return true;
}

bool SOAToAOSCFGInfo::populateCFGInformation(Module &M,
                                             bool RespectSizeHeuristic,
                                             bool RespectAttrs) {

  // Cannot process declarations and varargs.
  auto IsValidMethod = [RespectAttrs](Function &F) -> bool {
    if ((RespectAttrs && F.isDeclaration()) || F.isVarArg())
      return false;
    return true;
  };

  // Initialize fields' methods to empty sets.
  ArrayFieldsMethods.assign(getNumArrays(), MethodSetTy());

  for (auto &F : M) {
    // Skip unused prototypes.
    if (F.isDeclaration() && F.use_empty())
      continue;
    if (auto *ThisTy = getStructTypeOfMethod(F)) {
      if (ThisTy == Struct) {

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

      for (auto &Arg : F->args()) {
        auto *Ty = Arg.getType();
        if (Ty->isIntegerTy())
          continue;
        // By-value argument of element type.
        if (Ty == std::get<1>(Triple))
          continue;

        if (!Ty->isPointerTy())
          return FALSE("array method has unexpected non-pointer parameter.");

        if (Ty->getPointerAddressSpace())
          return FALSE("array method has non-0 address space pointer.");

        auto *Pointee = Ty->getPointerElementType();
        if (Pointee == MemoryInterface)
          continue;

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
          case Attribute::NoUndef:
            continue;
          default:
            if (Arg.hasAttribute(Kind))
              return FALSE("array method has unexpected parameter attribute");
          }
        }

        // Only MemoryInterface and by-value argument of element type can be
        // captured.
        // Ctor/Dtor/CCtor have return types on windows and “this”
        // parameter is marked with “Returned”.
        if (RespectAttrs &&
            !(Arg.hasNoCaptureAttr() || Arg.hasReturnedAttr())) {
          return FALSE("array method captures unexpected parameter.");
        }

        // See list of methods supported.
        if (Pointee != std::get<1>(Triple) && Pointee != std::get<2>(Triple))
          return FALSE("array method has unsupported parameter.");
      }
    }
  }

  DEBUG_WITH_TYPE(DTRANS_LAYOUT_DEBUG_TYPE, {
    dbgs() << "  ; Struct's " << Struct->getName() << " methods:\n";
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

} // namespace soatoaos
} // namespace dtrans
} // namespace llvm
