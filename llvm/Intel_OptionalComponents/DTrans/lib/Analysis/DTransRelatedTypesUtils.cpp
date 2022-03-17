//===--------------------DTransRelatedTypesUtils.cpp----------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//
//
// This file contains the implementation of the helper class
// DTransRelatedTypesUtils. This class contains a set of utility functions
// used for collecting and processing the structures with ABI padding. For
// example, assume that the module contains the following two structures:
//
//   %struct.test.a = type {i32, [4 x i8]}
//   %struct.test.a.base = type {i32}
//
// This class will be used to determine if the structure %struct.test.a is
// used for ABI padding and %struct.test.a.base is the base version.

#include "Intel_DTrans/Analysis/DTransRelatedTypesUtils.h"

#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/PtrTypeAnalyzer.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"

using namespace llvm;
using namespace dtransOP;

// Enable merging padded structures with base structures even if the
// safety checks didn't pass. This option is for testing purposes and
// must remain turned off by default.
static cl::opt<bool> DTransTestPaddedStructs(
    "dtrans-test-padded-structs-analyzer", cl::init(false), cl::ReallyHidden,
    cl::desc("Force merging padded structures with base structures even if "
             "the safety checks didn't pass"));

// Build the related types map. This map will store the relationship between
// a padded structure and a base structure.
DTransRelatedTypesUtils::DTransRelatedTypesUtils(DTransTypeManager &TM) {
  for (auto* DTy : TM.dtrans_types()) {
    if (RelatedTypesMap.count(DTy) > 0)
      continue;

    llvm::Type *Ty = DTy->getLLVMType();
    if (!isa<llvm::StructType>(Ty))
      continue;

    auto *DTRelatedTy = collectRelatedDTransType(DTy, TM);
    if (!DTRelatedTy)
      continue;

    RelatedTypesMap[DTRelatedTy] = DTy;
    RelatedTypesMap[DTy] = DTRelatedTy;
  }
}

// Return the related type for the given input type if it exists, else
// return nullptr.
DTransType* DTransRelatedTypesUtils::getRelatedTypeFor(DTransType *DTTy) {
  if (!DTTy || RelatedTypesMap.empty())
    return nullptr;

  auto It = RelatedTypesMap.find(DTTy);
  if (It == RelatedTypesMap.end())
    return nullptr;

  return It->second;
}

// Go through the multiple StructInfos and check if there is a safety
// violation that makes the padded field dirty. Also, this is the moment
// where we are going to set which structures are base and padded structures.
void DTransRelatedTypesUtils::postProcessRelatedTypesAnalysis(
    DTransSafetyInfo &DTInfo) {

  if (RelatedTypesMap.empty())
    return;

  // First check if the related types can be set or split
  finalizeBaseAndPaddedStructures(DTInfo);

  // Revert any safety violation
  revertSafetyData(DTInfo);
}

// Go through all the StructInfos and check if there is a safety violation that
// makes the padded field dirty. This is the moment where we are going to set
// which structures are base and padded structures.
void DTransRelatedTypesUtils::finalizeBaseAndPaddedStructures(
    DTransSafetyInfo &DTInfo) {
  // Return true if the input StructInfo has a padded field and
  // we found any safety violation for that field.
  auto HasInvalidPaddedField = [](dtrans::StructInfo *StrInfo) {
    if (!StrInfo)
      return true;
    if (!StrInfo->hasPaddedField())
      return false;
    size_t NumFields = StrInfo->getNumFields();
    auto &PaddedField = StrInfo->getField(NumFields - 1);
    if (!PaddedField.isPaddedField())
      return false;


    if (StrInfo->testSafetyData(dtrans::BadMemFuncManipulation))
      return true;

    // If the field is written or not top alloc function, then check if the
    // information was set due to BadMemFuncManipulationForRelatedTypes.
    //
    // NOTE: If the actual field reserved for padding is written, then there
    // is a chance that it is marked as Written. On the other hand, this field
    // is an array, which means that there might be a pointer for this field.
    // Most likely the field may be marked address taken or has complex use.
    if (PaddedField.isWritten() || !PaddedField.isTopAllocFunction()) {
      if (!StrInfo->testSafetyData(
          dtrans::BadMemFuncManipulationForRelatedTypes))
        return true;
    }

    // These are the conditions we use to invalidate the padded field.
    // Perhaps some of them could be relaxed like address taken and
    // complex use.
    if (PaddedField.isRead() || PaddedField.hasComplexUse() ||
        PaddedField.isAddressTaken())
      return true;

    return false;
  };

  // Convert the input safety data FromSD to ToSD
  auto ConvertSafetyData =
      [](dtrans::StructInfo *StructTI, dtrans::SafetyData FromSD,
         dtrans::SafetyData ToSD) -> void {

    if (!StructTI->testSafetyData(FromSD))
      return;

    StructTI->resetSafetyData(FromSD);
    StructTI->setSafetyData(ToSD);
  };

  // List of safety violations that needs to be reverted.
  //
  // NOTE: Would be best to put this information in a helper class. That way
  // we only need to update that helper class only.
  auto RevertSafetyDataToOriginal =
      [&ConvertSafetyData](dtrans::StructInfo *StructTI) {
    ConvertSafetyData(StructTI, dtrans::BadCastingForRelatedTypes,
                      dtrans::BadCasting);
    ConvertSafetyData(StructTI, dtrans::BadMemFuncManipulationForRelatedTypes,
                      dtrans::BadMemFuncManipulation);
  };

  const dtrans::SafetyData ABIPaddingSet =
      dtrans::StructCouldHaveABIPadding |
      dtrans::StructCouldBeBaseABIPadding;

  // NOTE: A dirty padded field means that we don't have enough information
  // to make sure if the field is not being modified.
  for (auto *TI : DTInfo.type_info_entries()) {
    if (auto *STInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      dtrans::StructInfo *RelatedTypeInfo = STInfo->getRelatedType();
      if (!RelatedTypeInfo)
        continue;
      // Skip those the structures that were analyzed already
      if (STInfo->testSafetyData(ABIPaddingSet))
        continue;
      // Identify the base and padded structures
      dtrans::StructInfo *BaseStruct = nullptr;
      dtrans::StructInfo *PaddedStruct = nullptr;
      if (STInfo->isABIPaddingPaddedStructure() &&
          RelatedTypeInfo->isABIPaddingBaseStructure()) {
        PaddedStruct = STInfo;
        BaseStruct = RelatedTypeInfo;
      } else if (STInfo->isABIPaddingBaseStructure() &&
          RelatedTypeInfo->isABIPaddingPaddedStructure()) {
        PaddedStruct = RelatedTypeInfo;
        BaseStruct = STInfo;
      } else {
        llvm_unreachable("Incorrect base and padded structures set");
      }
      // Check if the padded field has any safety violation that
      // could break the relationship.
      bool BadSafetyData = HasInvalidPaddedField(PaddedStruct);
      if (!BadSafetyData) {
        // The conditional safety violations should be the same between base
        // and padded structures.
        //
        // NOTE: This is conservative, perhaps we may need to relax it in the
        // future.
        dtrans::SafetyData RelatedConditions =
            dtrans::BadCastingForRelatedTypes |
            dtrans::BadMemFuncManipulationForRelatedTypes;

        bool PaddedData = PaddedStruct->testSafetyData(RelatedConditions);
        bool BaseData = BaseStruct->testSafetyData(RelatedConditions);

        BadSafetyData = PaddedData != BaseData;
      }

      if (BadSafetyData) {
        size_t NumFields = PaddedStruct->getNumFields();
        auto &Field = PaddedStruct->getField(NumFields - 1);
        Field.invalidatePaddedField();
      }
      // DTransTestPaddedStructs is used for testing purposes.
      if (!BadSafetyData || DTransTestPaddedStructs) {
        PaddedStruct->setSafetyData(dtrans::StructCouldHaveABIPadding);
        BaseStruct->setSafetyData(dtrans::StructCouldBeBaseABIPadding);
      } else {
        // If the safety data fails then break the relationship between
        // padded and base.
        PaddedStruct->unsetRelatedType();
        BaseStruct->unsetRelatedType();

        RevertSafetyDataToOriginal(PaddedStruct);
        RevertSafetyDataToOriginal(BaseStruct);
      }
    }
  }
}

// Traverse through each structure and check if the safety info contains any
// violation that is conditional to related types. For example, assume that
// we have the following structures:
//
//   %struct.test.a = type {i32, [4 x i8]}
//   %struct.test.a.base = type {i32}
//   %struct.test.b = type {%struct.test.a.base, [4 x i8]}
//
// And assume that %struct.test.a and %struct.test.a.base are set as
// "BadCasting" because the post processing process found that they aren't
// related types. Also asume that %struct.test.b was set as
// "BadCastingForRelatedTypes". This function will traverse through the
// zero field of %struct.test.b, identify that "BadCasting" was set in
// %struct.test.a.base, and will update "BadCastingForRelatedTypes"
// to "BadCasting".
void DTransRelatedTypesUtils::revertSafetyData(
    DTransSafetyInfo &DTInfo) {

  // List will be expanded as we add more conditions
  dtrans::SafetyData AllRelatedTypesSafety =
      dtrans::BadCastingForRelatedTypes |
      dtrans::BadMemFuncManipulationForRelatedTypes;

  for (auto *TI : DTInfo.type_info_entries()) {
    dtrans::SafetyData DataToRevert = dtrans::NoIssues;
    if (auto *STInfo = dyn_cast<dtrans::StructInfo>(TI)) {
      if (!STInfo->testSafetyData(AllRelatedTypesSafety))
        continue;

      auto *DTStruct = cast<DTransStructType>(STInfo->getDTransType());
      // Traverse through the zero field and check if it leads to a
      // safety bit of interest.
      auto *CurrStruct = DTStruct;
      SetVector<DTransType *> VisitedTypes;
      while (CurrStruct) {
        if (CurrStruct->getNumFields() == 0) {
          DataToRevert = AllRelatedTypesSafety;
          break;
        }

        // Recursive structures aren't allowed at the moment. This
        // is conservative.
        if (!VisitedTypes.insert(cast<DTransType>(CurrStruct))) {
          DataToRevert = AllRelatedTypesSafety;
          break;
        }

        auto *CurrStructInfo = DTInfo.getTypeInfo(CurrStruct);

        // If there is a BadCasting in the chain then there is no
        // point to use BadCastingForRelatedTypes
        if (CurrStructInfo->testSafetyData(dtrans::BadCasting))
          DataToRevert |= dtrans::BadCastingForRelatedTypes;

        // If there is a BadMemFuncManipulation in the chain then there is no
        // point to use BadMemFuncManipulationForRelatedTypes
        if (CurrStructInfo->testSafetyData(dtrans::BadMemFuncManipulation))
          DataToRevert |= dtrans::BadMemFuncManipulationForRelatedTypes;

        // If we need to convert all the safety violations then there is
        // nothing else to look for
        if (DataToRevert == AllRelatedTypesSafety)
          break;

        // NOTE: We currently support nested structures, perhaps in the future
        // we may want to extend the analysis when the field if a pointer to
        // a structure.
        DTransType *FieldZero = CurrStruct->getFieldType(0);
        if (auto *NewStruct = dyn_cast<DTransStructType>(FieldZero))
          CurrStruct = NewStruct;
        else
          CurrStruct = nullptr;
      }

      if (DataToRevert != dtrans::NoIssues) {
        VisitedTypes.clear();
        convertSafetyData(DTInfo, DTStruct, VisitedTypes, DataToRevert);
      }
    }
  }
}

// Find the DTrans TypeInfo for the input DTransType and replace the
// safety violations for related type into the original type if the bit
// is enabled in DataToRevert. In other words, this is the place where
// BadCastingForRelatedTypes is converted into BadCasting if something
// went wrong.
void DTransRelatedTypesUtils::convertSafetyData(DTransSafetyInfo &DTInfo,
    DTransType *DTTy, SetVector<DTransType *> &VisitedTypes,
    dtrans::SafetyData DataToRevert) {

  // Convert the SafetyData FromSD to ToSD if FromSD is set in TI and
  // is in DataToRevert
  auto ReplaceSafetyData =
      [DataToRevert](dtrans::TypeInfo *TI, dtrans::SafetyData FromSD,
                     dtrans::SafetyData ToSD) -> void {

    if (FromSD & DataToRevert) {
      if (!TI->testSafetyData(FromSD))
        return;

      TI->resetSafetyData(FromSD);
      TI->setSafetyData(ToSD);
    }
  };

  // Handle all the safety bits for related types that we want to reset.
  // List can be expanded as we update the related types analysis
  auto ListOfSafetyDataFunc =
    [&ReplaceSafetyData](dtrans::TypeInfo *TI) -> void {
    ReplaceSafetyData(TI, dtrans::BadCastingForRelatedTypes,
                      dtrans::BadCasting);
    ReplaceSafetyData(TI, dtrans::BadMemFuncManipulationForRelatedTypes,
                      dtrans::BadMemFuncManipulation);
  };

  DTransType *CurrTy = DTTy;
  while (CurrTy->isPointerTy() || CurrTy->isVectorTy()) {
    if (CurrTy->isPointerTy())
      CurrTy = CurrTy->getPointerElementType();
    else
      CurrTy = CurrTy->getVectorElementType();
  }

  if (!VisitedTypes.insert(CurrTy))
    return;

  if (!CurrTy->isAggregateType())
    return;

  dtrans::TypeInfo *TI = DTInfo.getTypeInfo(CurrTy);

  // Check which bits need to be updated.
  ListOfSafetyDataFunc(TI);

  // If the current TypeInfo is a structure and it has a related type then
  // we need to update it.
  if (auto *StructTI = dyn_cast<dtrans::StructInfo>(TI)) {
    if (auto *RelatedTI = StructTI->getRelatedType())
      ListOfSafetyDataFunc(RelatedTI);
  }

  // Cascade the conversion
  if (auto *StructTy = dyn_cast<DTransStructType>(CurrTy)) {
    for (auto Field : StructTy->elements()) {
      DTransType *FieldTy = Field.getType();
      if (!FieldTy)
        continue;

      convertSafetyData(DTInfo, FieldTy, VisitedTypes, DataToRevert);
    }
  } else if (auto *ArrTy = dyn_cast<DTransArrayType>(CurrTy)) {
    DTransType *ElemType = ArrTy->getArrayElementType();
    convertSafetyData(DTInfo, ElemType, VisitedTypes, DataToRevert);
  }
}

// Given a DTransType, find the related type from the input DTransTypeManager.
// For example, assume that InTy is a base type:
//
// %class.A.base = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32 }>
//
// This function will find the padded form from the input module:
//
// %class.A = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32, [4 x i8] }>
//
// It also works the other way around, given a padded structure InTy it will
// find the base form.
//
// NOTE: This is the same function as dtrans::collectRelatedType, but it uses
// DTransType instead of llvm::Type.
DTransType* DTransRelatedTypesUtils::collectRelatedDTransType(
    DTransType *InTy, DTransTypeManager &TM) {
  DTransStructType *CurrStruct = dyn_cast<DTransStructType>(InTy);
  if (!CurrStruct)
    return nullptr;

  if (CurrStruct->isLiteralStruct())
    return nullptr;

  StringRef StructName = CurrStruct->getName();
  std::string StrRelatedName;
  // Generate the type's name that we need to find in the module.
  if (StructName.endswith(".base")) {
    // Input type is a base type (%class.A.base), generate the
    // padded type name (%class.A)
    StrRelatedName = StructName.drop_back(5).str();
  } else {
    // Input type is a padded type (%class.A), generate the base
    // type name (%class.A.base)
    StrRelatedName = StructName.str() + ".base";
  }

  // FIXME: There could be cases, where the base name doesn't match.
  // For example:
  //
  //   %class.A = type opaque
  //   %class.A.1 = type <{ ptr, i32, i8, [3 x i8] }>
  //   %class.A.base = type <{ ptr, float, i8 }>
  //   %class.A.base.2 = type <{ ptr, i32, i8 }>
  //
  // This issue happens when templates are involved in the source code.
  DTransStructType* RelatedType = TM.getStructType(StrRelatedName);
  if (!RelatedType)
    return nullptr;
  if (!isPaddedDTransStruct(InTy, RelatedType))
    return nullptr;

  return RelatedType;
}

// Return true if the input DTransType Type1 is the same as DTransType Type2
// except for the last element (or vice versa). For example, consider that
// there is a class A which will be a base class for other derived classes and
// there is an instantiation of A. Then we will see something like the
// following in the IR:
//
// %class.A.base = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32 }>
// %class.A = type <{ %"class.boost::array", [2 x i8],
//                         %"class.std::vector", i32, [4 x i8] }>
//
// The structure type %class.A.base is the same as %class.A, except for the last
// entry of structure %class.A. This entry at the end ([4 x i8]) is used for
// the application binary interface (ABI).
bool DTransRelatedTypesUtils::isPaddedDTransStruct(DTransType *Type1,
                                                   DTransType *Type2) {
  if (!Type1 || !Type2)
    return false;

  if (!Type1->isStructTy() || !Type2->isStructTy())
    return false;

  unsigned Type1Size = Type1->getNumContainedElements();
  unsigned Type2Size = Type2->getNumContainedElements();
  unsigned PaddedSize = 0;
  unsigned BaseSize = 0;
  if (Type1Size == 0 || Type2Size == 0)
    return false;

  DTransStructType *BaseStruct = nullptr;
  DTransStructType *PaddedStruct = nullptr;
  // Find the possible base and the padded types
  if (Type1Size - Type2Size == 1) {
    PaddedStruct = cast<DTransStructType>(Type1);
    PaddedSize = Type1Size;
    BaseStruct = cast<DTransStructType>(Type2);
    BaseSize = Type2Size;
  } else if (Type2Size - Type1Size == 1) {
    BaseStruct = cast<DTransStructType>(Type1);
    BaseSize = Type1Size;
    PaddedStruct = cast<DTransStructType>(Type2);
    PaddedSize = Type2Size;
  } else {
    return false;
  }

  if (PaddedStruct->isLiteralStruct() || BaseStruct->isLiteralStruct())
    return false;

  DTransArrayType *PaddedEntry =
      dyn_cast<DTransArrayType>(PaddedStruct->getFieldType(PaddedSize - 1));
  // Check if the current structure is a candidate for padded structure
  if (!PaddedEntry)
    return false;

  auto* PaddedEntryLLVMType =
      cast<llvm::ArrayType>(PaddedEntry->getLLVMType());
  if (!PaddedEntryLLVMType->getElementType()->isIntegerTy(8))
    return false;

  StringRef PaddedName = PaddedStruct->getName();
  StringRef BaseName = BaseStruct->getName();
  if (!BaseName.endswith(".base"))
    return false;

  // FIXME: There could be cases where the base name doesn't match.
  // For example:
  //
  //   %class.A = type opaque
  //   %class.A.1 = type <{ ptr, i32, i8, [3 x i8] }>
  //   %class.A.base = type <{ ptr, float, i8 }>
  //   %class.A.base.2 = type <{ ptr, i32, i8 }>
  //
  // This issue happens when templates are involved in the source code.
  if (BaseName.compare(PaddedName.str() + ".base") != 0)
    return false;

  // All the elements must match except the last one;
  for (unsigned Element = 0; Element < BaseSize; Element++) {
    auto *PaddedStructField = PaddedStruct->getFieldType(Element);
    auto *BaseStructField = BaseStruct->getFieldType(Element);
    if (!PaddedStructField->compare(*BaseStructField))
      return false;
  }

  return true;
}
