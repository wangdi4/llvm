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

  // Go through each structure and make a decision if the safety data
  // that is conditional should be set for related types or not
  void lowerConditionalSafetyData(DTransSafetyInfo &DTInfo);

  // Convert the conditional safety data into the related types version
  void convertSafetyData(DTransSafetyInfo &DTInfo, DTransType *DTTy,
                         SetVector<DTransType *> &VisitedTypes,
                         bool ForRelatedTypes);
};

} // end namespace dtransOP

} // end namespace llvm
#endif // INTEL_DTRANS_ANALYSIS_DTRANSRELATEDTYPESUTILS_H