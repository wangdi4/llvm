# INTEL_CUSTOMIZATION
set(INTEL_SOURCE_FILES
  Intel_AggInlAA.cpp
  Intel_GlobalOpt.cpp
  Intel_IndirectCallConv.cpp
  Intel_LoopOptMarker.cpp
  Intel_LoopOptReportEmitter.cpp
  Intel_LowerSubscriptIntrinsic.cpp
  Intel_RemoveRegionDirectives.cpp
  Intel_StdContainerOpt.cpp
  Intel_TbaaMDPropagation.cpp
  Intel_MultiVersioning.cpp
  )

set(INTEL_CSA_SOURCE_FILES
  LoopSPMDization.cpp
  )

set(INTEL_SOURCE_FILES_TO_BUILD)
if (INTEL_CUSTOMIZATION)
#  if(";${LLVM_TARGETS_TO_BUILD};" MATCHES ";CSA;")
    list(APPEND INTEL_SOURCE_FILES ${INTEL_CSA_SOURCE_FILES})
#  else()
#    list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_CSA_SOURCE_FILES})
#  endif()

  set(INTEL_SOURCE_FILES_TO_BUILD ${INTEL_SOURCE_FILES})
else()
  list(APPEND LLVM_OPTIONAL_SOURCES
    ${INTEL_SOURCE_FILES}
    ${INTEL_CSA_SOURCE_FILES}
    )
endif()
# end INTEL_CUSTOMIZATION

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
# INTEL_CUSTOMIZATION
  ${INTEL_SOURCE_FILES_TO_BUILD}
# end INTEL_CUSTOMIZATION
  JumpThreading.cpp
  LICM.cpp
  LoopAccessAnalysisPrinter.cpp
  LoopSink.cpp
  LoopDeletion.cpp
  LoopDataPrefetch.cpp
  LoopDistribute.cpp
  LoopIdiomRecognize.cpp
  LoopInterchange.cpp
  LoopLoadElimination.cpp
  LoopPassManager.cpp
  LoopPredication.cpp
  LoopRerollPass.cpp
  LoopRotation.cpp
  LoopSimplifyCFG.cpp
  LoopStrengthReduce.cpp
  LoopUnrollPass.cpp
  LoopUnswitch.cpp
  LoopVersioningLICM.cpp
  LowerAtomic.cpp
  LowerExpectIntrinsic.cpp
  LowerGuardIntrinsic.cpp
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

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Scalar

  DEPENDS
  intrinsics_gen
  )

# INTEL_CUSTOMIZATION
if(INTEL_INCLUDE_DTRANS)
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_DTransAnalysis)
endif(INTEL_INCLUDE_DTRANS)
# end INTEL_CUSTOMIZATION
