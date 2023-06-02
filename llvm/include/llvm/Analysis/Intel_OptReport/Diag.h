//===------ Diag.h - Diagnostics for HIR -------*- C++ -*---------===//
//
// Copyright (C) 2015-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//
// This file defines the diagnostics in high level IR.
//
// This is a temporary solution until we move to the diagnostics
// mechanism using derived classes from DiagnosticInfo.
//
//===--------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
#define LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H

#include "llvm/ADT/DenseMap.h"

namespace llvm {

enum class OptRemarkID {
  /// ID to represent invalid remarks i.e. remarks which are not present in
  /// Diags table.
  InvalidRemarkID = 0,

  /// Named constants for commonly used remarks.  This will be converted
  /// to an enum class after we have identifiers for all remark IDs.

  /// Vectorizer remarks from 15300-15552 match those in ICC.
  LoopVectorized = 15300,
  SimdLoopVectorized = 15301,
  VectorizationFactor = 15305,
  NormalizedVecOverhead = 15309,
  VecFailBadType = 15313,
  VecFailLowTripCount = 15315,
  VecFailBadReduction = 15330,
  VecFailUserExcluded = 15332,
  VecFailNotProfitable = 15335,
  VecFailBadlyFormedSimdLoop = 15353,
  VectorizerUnrollFactor = 15399,
  VecFailBadComplexFloatOp = 15407,
  VecFailBadComplexDoubleOp = 15408,
  VecFailGenericBailout = 15436,
  VectorizedPeelLoop = 15437,
  VectorizedRemainderLoopUnmasked = 15439,
  VectorizedRemainderLoopMasked = 15440,
  UnvectorizedRemainderLoop = 15441,
  BeginVectorLoopMemRefSummary = 15447,
  UnmaskedUnalignedUnitStrideLoads = 15450,
  UnmaskedUnalignedUnitStrideStores = 15451,
  MaskedUnalignedUnitStrideLoads = 15456,
  MaskedUnalignedUnitStrideStores = 15457,
  MaskedGathers = 15458,
  MaskedScatters = 15459,
  UnmaskedGathers = 15462,
  UnmaskedScatters = 15463,
  EndVectorLoopMemRefSummary = 15474,
  BeginVectorLoopCostSummary = 15475,
  VectorizerScalarLoopCost = 15476,
  VectorizerVectorLoopCost = 15477,
  VectorizerEstimatedSpeedup = 15478,
  VectorizedMathLibCalls = 15482,
  VectorFunctionCalls = 15484,
  SerializedFunctionCalls = 15485,
  EndVectorLoopCostSummary = 15488,
  VectorCompressStores = 15497,
  VectorExpandLoads = 15498,
  VectorizerLoopNumber = 15506,
  VecFailBadlyFormedMultiExitLoop = 15520,
  VecFailUnknownInductionVariable = 15521,
  VecFailComplexControlFlow = 15522,
  VecFailSwitchPresent = 15535,
  UnmaskedVLSLoads = 15554,
  MaskedVLSLoads = 15555,
  UnmaskedVLSStores = 15556,
  MaskedVLSStores = 15557,

  /// Vectorizer remarks that are xmain-specific.
  FirstSerializationRemark = 15558,
  CallSerializedNoVecVariants = 15558,
  CallSerializedIndirectCall = 15560,
  LoadStoreSerializedBadType = 15563,
  ExtractInsertSerialized = 15564,
  MaskedExtractInsertSerialized = 15565,
  DivisionSerializedFpModel = 15566,
  GatherReason = 15567,
  ScatterReason = 15568,
  VectorizerShortVector = 15569,
  VectorizerScalarTripCount = 15570,
  VecFailUnknownRecurrence = 15571,
  VecFailUnknownLiveOut = 15572,
  VecFailReducingVectorType = 15573,
  VecFailNestedSimdRegion = 15574,
  VectorizerStaticPeeling = 15575,
  VectorizerDynamicPeeling = 15576,
  VectorizerEstimatedPeelIters = 15577,
  GenericDebug = 15578,

  /// Loop optimization remarks start at 25045.  Some of these are
  /// also used by the vectorizer.
  VectorizerPeelLoop = 25518,
  VectorizerRemainderLoop = 25519,
  LoopHasReduction = 25587,
  LoopHasSimdReduction = 25588,
};

struct DiagTableKey {
  OptRemarkID ID;
  DiagTableKey(OptRemarkID ID) : ID(ID) {}
  DiagTableKey(unsigned ID) : ID(static_cast<OptRemarkID>(ID)) {}
  operator unsigned() const { return static_cast<unsigned>(ID); }
};

// Use DiagTableKey::operator unsigned() as a shortcut to define
// DenseMapInfo for DiagTableKey.
template <> struct DenseMapInfo<DiagTableKey> : DenseMapInfo<unsigned> {};

class OptReportDiag {
  static const DenseMap<DiagTableKey, const char *> Diags;

public:
  /// Retrieve message string from the diagnostic ID.
  static const char *getMsg(DiagTableKey Id);
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
