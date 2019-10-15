add_llvm_library(LLVMScalarOpts
  ADCE.cpp
  AlignmentFromAssumptions.cpp
  BDCE.cpp
  CallSiteSplitting.cpp
  ConstantHoisting.cpp
  ConstantProp.cpp
  CorrelatedValuePropagation.cpp
  DCE.cpp
  DeadStoreElimination.cpp
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
  Intel_GlobalOpt.cpp
  Intel_IndirectCallConv.cpp
  Intel_LoopOptMarker.cpp
  Intel_LoopOptReportEmitter.cpp
  Intel_LowerSubscriptIntrinsic.cpp
  Intel_RemoveRegionDirectives.cpp
  Intel_StdContainerOpt.cpp
  Intel_TbaaMDPropagation.cpp
  Intel_TransformFPGAReg.cpp
  Intel_MultiVersioning.cpp
  Intel_AddSubReassociate.cpp
  Intel_LoopCarriedCSE.cpp
  IVSplit.cpp
# end INTEL_CUSTOMIZATION
  JumpThreading.cpp
  LICM.cpp
  LoopAccessAnalysisPrinter.cpp
  LoopSink.cpp
  LoopDeletion.cpp
  LoopDataPrefetch.cpp
  LoopDistribute.cpp
  LoopFuse.cpp
  LoopIdiomRecognize.cpp
  LoopInstSimplify.cpp
  LoopInterchange.cpp
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
  SeparateConstOffsetFromGEP.cpp
  SimpleLoopUnswitch.cpp
  SimplifyCFGPass.cpp
  Sink.cpp
  SpeculativeExecution.cpp
  SpeculateAroundPHIs.cpp
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
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_DTransAnalysis)
endif(INTEL_INCLUDE_DTRANS)
# INTEL_FEATURE_CSA
is_intel_feature_enabled(p ${LLVM_INTELFEATURE_PREFIX}_CSA)
if(p)
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_CSAScalarOpt)
endif()
# end INTEL_FEATURE_CSA
# end INTEL_CUSTOMIZATION
