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
  LoopUnswitch.cpp
  LoopVersioningLICM.cpp
  LowerAtomic.cpp
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
