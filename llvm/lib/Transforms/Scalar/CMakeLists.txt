# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
add_llvm_component_library(LLVMScalarOpts
  ADCE.cpp
  AlignmentFromAssumptions.cpp
  AnnotationRemarks.cpp
  BDCE.cpp
  CallSiteSplitting.cpp
  ConstantHoisting.cpp
  ConstraintElimination.cpp
  CorrelatedValuePropagation.cpp
  DCE.cpp
  DeadStoreElimination.cpp
  DFAJumpThreading.cpp
  DivRemPairs.cpp
  EarlyCSE.cpp
  FlattenCFGPass.cpp
  Float2Int.cpp
  GuardWidening.cpp
  GVN.cpp
  GVNHoist.cpp
  GVNSink.cpp
  IVUsersPrinter.cpp
  InductiveRangeCheckElimination.cpp
  IndVarSimplify.cpp
  InferAddressSpaces.cpp
  InstSimplifyPass.cpp
# INTEL_CUSTOMIZATION
  Intel_DopeVectorHoist.cpp
#if INTEL_FEATURE_SW_ADVANCED
  Intel_FunctionRecognizer.cpp
#endif // INTEL_FEATURE_SW_ADVANCED
  Intel_GlobalOpt.cpp
  Intel_HandlePragmaVectorAligned.cpp
  Intel_IndirectCallConv.cpp
  Intel_LoopAttrs.cpp
  Intel_LoopOptMarker.cpp
  Intel_OptReportEmitter.cpp
  Intel_LowerSubscriptIntrinsic.cpp
# INTEL_FEATURE_SW_ADVANCED
  Intel_NontemporalStore.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_RemoveRegionDirectives.cpp
  Intel_StdContainerOpt.cpp
# INTEL_FEATURE_SW_ADVANCED
  Intel_TightLoopEmitter.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_TbaaMDPropagation.cpp
  Intel_TransformFPGAReg.cpp
  Intel_TransformSinAndCosCalls.cpp
  Intel_MultiVersioning.cpp
  Intel_AddSubReassociate.cpp
  Intel_ForcedCMOVGeneration.cpp
  Intel_LoopCarriedCSE.cpp
  Intel_IVSplit.cpp
# end INTEL_CUSTOMIZATION
  JumpThreading.cpp
  LICM.cpp
  LoopAccessAnalysisPrinter.cpp
  LoopBoundSplit.cpp
  LoopSink.cpp
  LoopDeletion.cpp
  LoopDataPrefetch.cpp
  LoopDistribute.cpp
  LoopFuse.cpp
  LoopIdiomRecognize.cpp
  LoopInstSimplify.cpp
  LoopInterchange.cpp
  LoopFlatten.cpp
  LoopLoadElimination.cpp
  LoopPassManager.cpp
  LoopPredication.cpp
  LoopRerollPass.cpp
  LoopRotation.cpp
  LoopSimplifyCFG.cpp
  LoopStrengthReduce.cpp
  LoopUnrollPass.cpp
  LoopUnrollAndJamPass.cpp
# INTEL_CUSTOMIZATION
  Intel_LegacyLoopUnswitch.cpp
# end INTEL_CUSTOMIZATION
  LoopVersioningLICM.cpp
  LowerAtomicPass.cpp
  LowerConstantIntrinsics.cpp
  LowerExpectIntrinsic.cpp
  LowerGuardIntrinsic.cpp
  LowerMatrixIntrinsics.cpp
  LowerWidenableCondition.cpp
  MakeGuardsExplicit.cpp
  MemCpyOptimizer.cpp
  MergeICmps.cpp
  MergedLoadStoreMotion.cpp
  NaryReassociate.cpp
  NewGVN.cpp
  PartiallyInlineLibCalls.cpp
  PlaceSafepoints.cpp
  Reassociate.cpp
  Reg2Mem.cpp
  RewriteStatepointsForGC.cpp
  SCCP.cpp
  SROA.cpp
  Scalar.cpp
  Scalarizer.cpp
  ScalarizeMaskedMemIntrin.cpp
  SeparateConstOffsetFromGEP.cpp
  SimpleLoopUnswitch.cpp
  SimplifyCFGPass.cpp
  Sink.cpp
  SpeculativeExecution.cpp
  StraightLineStrengthReduce.cpp
  StructurizeCFG.cpp
  TailRecursionElimination.cpp
  TLSVariableHoist.cpp
  WarnMissedTransforms.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Scalar

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB

  COMPONENT_NAME
  Scalar

  LINK_COMPONENTS
  AggressiveInstCombine
  Analysis
  Core
  InstCombine
  Support
  TransformUtils
  )

# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_DTRANS
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_SW_DTRANS)
if (p)
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_DTransAnalysis)
endif()
# end INTEL_FEATURE_SW_DTRANS
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if(p)
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
