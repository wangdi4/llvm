//===------ Diag.h - Diagnostics for HIR -------*- C++ -*---------===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
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
#include "llvm/Support/raw_ostream.h"

namespace llvm {

enum class OptRemarkID {
  /// ID to represent invalid remarks i.e. remarks which are not present in
  /// Diags table. All remarks have been added to the Diags table now, so use of
  /// this value is an error.
  InvalidRemarkID = 0,

  /// Named constants for commonly used remarks.

  /// Vectorizer remarks from 15300-15552 match those in ICC.
  LoopVectorized = 15300,
  SimdLoopVectorized = 15301,
  VectorizationFactor = 15305,
  NormalizedVecOverhead = 15309,
  VecFailBadType = 15313,
  VecFailLowTripCount = 15315,
  VecFailNovectorDirective = 15319,
  VecFailBadReduction = 15330,
  VecFailUserExcluded = 15332,
  VecFailNotProfitable = 15335,
  VecFailVectorDependence = 15344,
  VectorDependence = 15346,
  VecFailBadlyFormedSimdLoop = 15353,
  VectorizerUnrollFactor = 15399,
  VecFailBadComplexFloatOp = 15407,
  VecFailBadComplexDoubleOp = 15408,
  VecFailLoopEmptyAfterOpt = 15414,
  VecLoopCompletelyUnrolled = 15427,
  VecFailGenericBailout = 15436,
  VectorizedPeelLoop = 15437,
  VectorizedRemainderLoopUnmasked = 15439,
  VectorizedRemainderLoopMasked = 15440,
  UnvectorizedRemainderLoop = 15441,
  BeginVectorLoopMemRefSummary = 15447,
  UnmaskedAlignedUnitStrideLoads = 15448,
  UnmaskedAlignedUnitStrideStores = 15449,
  UnmaskedUnalignedUnitStrideLoads = 15450,
  UnmaskedUnalignedUnitStrideStores = 15451,
  MaskedAlignedUnitStrideLoads = 15454,
  MaskedAlignedUnitStrideStores = 15455,
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
  VecFailFuncCallNoVec = 15527,
  VecFailSwitchPresent = 15535,
  VecFailCannotAutoVecOuterLoop = 15553,
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
  VecCloneVLAPresence = 15580,
  VecCloneVariantLegalization = 15581,
  VecCloneLinearUValUnoptimized = 15582,
  VectorizerReductionInfo = 15590,
  VectorizedIntrinsics = 15591,
  VecWindowsAtomic = 15592,
  VecFailFence = 15593,

  /// Prefetching remarks.
  TotalLinesPrefetched = 25018,
  NumSpatialPrefetches = 25019,
  NumIndirectPrefetches = 25033,

  /// Loop optimization remarks start at 25045.  Some of these are
  /// also used by the vectorizer.
  FusedLoops = 25045,
  LoopLostInFusion = 25046,
  DirectivePrefetchSpatialMemRef = 25147,
  DirectivePrefetchIndirectMemRef = 25150,
  LoopMultiversionedForDD = 25228,
  LoopPeeledUsingCondition = 25258,
  LoopOptimizedAwayUsingCondition = 25259,
  DeadLoopOptimizedAway = 25260,
  SingleIterationLoopOptimizedAway = 25261,
  LoopRerollFactor = 25264,
  MaterializedLoopTripCount = 25397,
  MemcopyGenerated = 25399,
  MemsetGenerated = 25408,
  InvariantConditionHoisted = 25422,
  InvariantIfConditionHoisted = 25423,
  InvariantSwitchConditionHoisted = 25424,
  LoopDistributionChunkNum = 25425,
  LoopDistributionPerfectNest = 25426,
  LoopDistributionEnableVec = 25427,
  LoopStripMineFactor = 25428,
  CompleteUnrollFactor = 25436,
  LoopUnrollFactorWithoutRemainder = 25438,
  LoopUnrollFactorWithRemainder = 25439,
  LoopNestInterchanged = 25444,
  LoopInterchangeFailReason = 25445,
  DependenciesBetweenStmts = 25446,
  LoopInterchange = 25447,
  AdviseLoopInterchange = 25451,
  LoopNestReplacedByMatmul = 25459,
  LoopMultiversioned = 25474,
  PredicateOptimized = 25476,
  WhileLoopUnrollFactor = 25478,
  DistributePointPragmaNotProcessed = 25481,
  NoDistributionAsRequested = 25482,
  DistributePointPragmaProcessed = 25483,
  DistribPragmaFailUnsupportedConstructs = 25484,
  DistribPragmaFailLoopNestTooLarge = 25485,
  DistribPragmaFailExcessDistribPoints = 25486,
  LoopPeeledForDataDependence = 25487,
  RemainderLoop = 25491,
  LoopCompletelyUnrolled = 25508,
  VectorizerPeelLoop = 25518,
  VectorizerRemainderLoop = 25519,
  MemoryReductionSinking = 25528,
  DeadStoresEliminated = 25529,
  StmtSunkAfterLoopLastValue = 25530,
  LoopUnrollAndJamFactor = 25540,
  LoopMultiversionedSmallTripCount = 25562,
  LoadHoistedFromLoop = 25563,
  LoadSunkFromLoop = 25564,
  BlockingUsingPragma = 25565,
  LoopBlockingFactor = 25566,
  NumCollapsedLoops = 25567,
  LoopReversed = 25579,
  IVarRangeSplitUsingCondition = 25580,
#if INTEL_INTERNAL_BUILD
  LoopRowWiseMultiversioned = 25581,
  RowWiseMultiversionedLoop = 25582,
#endif // INTEL_INTERNAL_BUILD
  NumArrayRefsScalarReplaced = 25583,
#if INTEL_INTERNAL_BUILD
  SumWindowReuseCount = 25584,
#endif // INTEL_INTERNAL_BUILD
  LoopConvertedToSwitch = 25585,
  PeeledLoopForFusion = 25586,
  LoopHasReduction = 25587,
  LoopHasSimdReduction = 25588,
  HoistedConditionalLoads = 25589,
  SunkConditionalStores = 25590,
  TightLoopFound = 25599,
  TightLoopValue = 25600,
#if INTEL_INTERNAL_BUILD
  RowWiseMultiversionProbeLoop = 25601,
  WindowSumInitialization = 25602,
#endif // INTEL_INTERNAL_BUILD
  LLORGFullyUnrolled = 25603,
  LLORGUnrolledBy = 25604,
  LLORGRemainderLoop = 25605,
  LLORGPeeledBy = 25606,

  // Paropt/OpenMP remarks start at 30000.
  OpenMPOutlinedParLoop = 30000,
  OpenMPOutlinedEnclosedParLoop = 30001,
  OpenMPWorkSharingLoop = 30002,
  OpenMPRedundantClause = 30003,
  OpenMPClauseHasBeenChanged = 30004,
  OpenMPClauseCanBeChanged = 30005,
#if INTEL_FEATURE_CSA
  OpenMPParLoopPipelined = 30006,
  OpenMPWorkShareLoopPipelined = 30007,
#endif // INTEL_FEATURE_CSA
  OpenMPConstructTransformed = 30008,
  OpenMPConstructUserIgnored = 30009,
  OpenMPConstructUnreachable = 30010,
  OpenMPConstructIgnored = 30011,
  OpenMPClangImplicitMap = 30012,
  OpenMPClangImplicitFirstPrivate = 30013,

  DummyRemarkForTesting = 99999,
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

inline raw_ostream &operator<<(raw_ostream &OS, OptRemarkID ID) {
  OS << static_cast<unsigned>(ID);
  return OS;
}

class OptReportDiag {
  static const DenseMap<DiagTableKey, const char *> Diags;

public:
  /// Retrieve message string from the diagnostic ID.
  static const char *getMsg(DiagTableKey Id);
};

enum class AuxRemarkID {
  InvalidAuxRemark,
  Empty,

  // Remark numbers through 10000 are reserved for the vectorizer.
  VectorizerRemarksBegin, // No associated message
  Loop,
  SimdLoop,
  OmpSimdOrderedUnsupported,
  VFNotPowerOf2,
  ForcedVFIs1,
  ForcedVFExceedsSafeLen,
  PragmaVectorLength0,
  OutOfVPlanVecRange,
  VFExceedsTC,
  ForcedVFExceedsUnrolledTC,
  UserForcedVF1,
  UDRWithoutInitializer,
  MultipleLiveOutReduction,
  IllegalOpenMPInSIMD,
  NoDedicatedExits,
  MultipleMultiExitLoops,
  OuterLoopVecUnsupported,
  DivergentBranchDisabled,
  VectorizerRemarksEnd, // No associated message

  // Remark numbers through 20000 are reserved for OpenMP.
  OpenMPRemarksBegin = 10001, // No associated message
  CapturedByLambda,
  CapturedInReferencedLambda,
  ThisKeywordReferenced,
  NonScalarFieldReferenced,
  PointerVariableReferenced,
  NonScalarVariableReferenced,
  OpenMPRemarksEnd, // No associated message
  // Add remark numbers for other components here.  Please reserve
  // remarks in blocks of 10000.
};

template <> struct DenseMapInfo<AuxRemarkID> {
  static inline AuxRemarkID getEmptyKey() {
    return static_cast<AuxRemarkID>(~0U);
  }
  static inline AuxRemarkID getTombstoneKey() {
    return static_cast<AuxRemarkID>(~0U - 1);
  }
  static unsigned getHashValue(const AuxRemarkID &Val) {
    return static_cast<unsigned>(Val) * 37U;
  }
  static bool isEqual(const AuxRemarkID &LHS, const AuxRemarkID &RHS) {
    return LHS == RHS;
  }
};

class OptReportAuxDiag {
  static const DenseMap<AuxRemarkID, const char *> AuxDiags;

public:
  /// Constructor for this singleton class that invokes all component
  /// verifiers.
  OptReportAuxDiag() { verifyVectorizerMsgs(); };

  /// Retrieve auxiliary message string from the diagnostic ID.
  static const char *getMsg(AuxRemarkID Id);

  /// Verify that the auxiliary diagnostic table contains a message for
  /// each vectorizer message.
  static void verifyVectorizerMsgs();
};

} // namespace llvm

#endif // LLVM_ANALYSIS_INTEL_OPTREPORT_DIAG_H
