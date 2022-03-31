//===-------------------DTransRelatedTypesUtils.h-------------------------===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

// This file defines the helper class DTransRelatedTypesUtils, which is used
// for collecting and processing the information related to structures with
// ABI padding.

#if !INTEL_FEATURE_SW_DTRANS
#error DTransRelatedTypesUtils.h include in an non-INTEL_FEATURE_SW_DTRANS\
       build.
#endif

#ifndef INTEL_DTRANS_ANALYSIS_DTRANSRELATEDTYPESUTILS_H
#define INTEL_DTRANS_ANALYSIS_DTRANSRELATEDTYPESUTILS_H

#include "Intel_DTrans/Analysis/DTrans.h"

namespace llvm {
namespace dtransOP {
class DTransSafetyInfo;
class DTransType;
class DTransTypeManager;

// Helper class for handling the related types' safety data and converting
// them into the original type.
class RelatedTypesSDHandler {
public:

  // Build the maps for handling the safety data for related types
  RelatedTypesSDHandler();

  // Return a safety data that represents all the safety violations for
  // related types
  dtrans::SafetyData getAllSafetyDataForRelatedTypes() {
    return AllRelatedTypesSafety;
  }

  // Given a StructInfo, compute all the safety violations available for
  // related types
  dtrans::SafetyData computeRelatedTypesSafetyData(dtrans::TypeInfo *TI);

  // Convert all the safety violations for related types available in the input
  // StructInfo into the original form
  void revertAllSafetyDataToOriginal(dtrans::TypeInfo *TI);

  // Helper function for reverting the safety violations in DataToRevert
  void convertSafetyData(DTransSafetyInfo &DTInfo, DTransType *DTTy,
                         dtrans::SafetyData DataToRevert);

private:
  DenseMap<dtrans::SafetyData, dtrans::SafetyData> RelatedSDToOriginalSDMap;
  dtrans::SafetyData AllRelatedTypesSafety = dtrans::NoIssues;

  // Convert the safety data from related types to original version and
  // cascade the safety data to the nested types.
  void convertSafetyDataCascade(DTransSafetyInfo &DTInfo, DTransType *DTTy,
                                SetVector<DTransType *> &VisitedTypes,
                                dtrans::SafetyData DataToRevert);

  // Convert all the safety violations for related types to the original form
  // in the input StructInfo if the safety data is available in SDToConvert
  void revertSafetyDataToOriginal(dtrans::TypeInfo *TI,
                                  dtrans::SafetyData SDToConvert);
};

// Helper class to handle all the analysis for Related types
class DTransRelatedTypesUtils {
public:
  // Build the related types map. This map will store the relationship between
  // a padded structure and a base structure.
  DTransRelatedTypesUtils(DTransTypeManager &TM);

  // Return the related type for the given input type if it exists, else
  // return nullptr.
  DTransType* getRelatedTypeFor(DTransType *DTTy);

  // Finalize the analysis for the base-padded structures and set the proper
  // safety information.
  void postProcessRelatedTypesAnalysis(DTransSafetyInfo &DTInfo);

  // Set input dtrans::StructInfos as related types
  void setTypeInfoAsRelatedTypes(dtrans::StructInfo *CurrTypeInfo,
                                 dtrans::StructInfo *RelatedTypeInfo);

private:
  // Map the types with ABI padding to the base types
  DenseMap<DTransType *, DTransType *> RelatedTypesMap;

  // Given a DTransType, find the related DTransType from the input
  // DTransTypeManager.
  DTransType* collectRelatedDTransType(DTransType *InTy,
                                       DTransTypeManager &TM);

  // Return true if the input type Type1 is the same as Type2 except for
  // the last element (or vice versa). This last element is used by the ABI.
  bool isPaddedDTransStruct(DTransType *Type1, DTransType *Type2);

  // Go through each structure and set which ones have ABI padding and which
  // ones are used as base.
  void finalizeBaseAndPaddedStructures(DTransSafetyInfo &DTInfo);

  // Go through the fields and check if the data for arrays with constant
  // entries matches.
  void analyzeFieldsWithArrayConstantEntries(dtrans::StructInfo *BaseStruct,
                                             dtrans::StructInfo *PaddedStruct);

  // Disable all the data for arrays with constant entries in the input
  // StructInfo
  void disableArraysWithConstantEntriesData(dtrans::StructInfo *STInfo);

  // Go through each structure and make a decision if the safety data
  // for related types needs to be reverted
  void revertSafetyData(DTransSafetyInfo &DTInfo);

  // Return true if the input StructInfo has a padded field and
  // we found any safety violation for that field.
  bool HasInvalidPaddedField(dtrans::StructInfo *StrInfo);

  // Handle the safety violations for related types.
  RelatedTypesSDHandler RTHandler;
};

} // end namespace dtransOP

} // end namespace llvm
#endif // INTEL_DTRANS_ANALYSIS_DTRANSRELATEDTYPESUTILS_H