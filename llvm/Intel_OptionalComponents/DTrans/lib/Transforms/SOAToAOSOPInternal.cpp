//===------------- SOAToAOSOPInternal.cpp - DTransSOAToAOSOPPass  ---------===//
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

#include "SOAToAOSOPInternal.h"

namespace llvm {
namespace dtransOP {
namespace soatoaosOP {

DTransStructType *getOPSOAArrayType(DTransStructType *Struct,
                                    unsigned Off) {
  return cast<DTransStructType>(
      Struct->getFieldType(Off)->getPointerElementType());
}

DTransPointerType *getOPSOAElementType(DTransStructType *ArrayType,
                                       unsigned BasePointerOffset) {
  return cast<DTransPointerType>(
      ArrayType->getFieldType(BasePointerOffset)->getPointerElementType());
}

DTransStructType *getOPStructTypeOfMethod(Function *F,
                                                 TypeMetadataReader &MDReader) {
  if (F->arg_size() < 1)
    return nullptr;

  DTransFunctionType *DFnTy = dyn_cast_or_null<DTransFunctionType>(
      MDReader.getDTransTypeFromMD(F));
  // Functions with no pointer types will be ignored.
  if (!DFnTy)
    return nullptr;

  if (auto *DPTy = dyn_cast<DTransPointerType>(DFnTy->getArgType(0)))
    if (auto *DSTy = dyn_cast<DTransStructType>(DPTy->getPointerElementType()))
      return DSTy;

  return nullptr;
}

DTransStructType *getOPStructTypeOfMethod(Function *F,
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

bool SOAToAOSOPLayoutInfo::populateLayoutInformation(DTransType *DTy) {

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
                                               TypeMetadataReader &MDReader,
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

    if (auto *ThisTy = getOPStructTypeOfMethod(&F, MDReader)) {
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
          MDReader.getDTransTypeFromMD(F));
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
          case Attribute::NoUndef:
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
