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

if (DEFINED LLVM_HAVE_TF_AOT OR LLVM_HAVE_TFLITE)
  include(TensorFlowCompile)
  set(LLVM_INLINER_MODEL_PATH_DEFAULT "models/inliner-Oz")

  set(LLVM_INLINER_MODEL_CURRENT_URL "<UNSPECIFIED>" CACHE STRING "URL to download the LLVM inliner model")

  if (DEFINED LLVM_HAVE_TF_AOT)
    tf_find_and_compile(
      ${LLVM_INLINER_MODEL_PATH}
      ${LLVM_INLINER_MODEL_CURRENT_URL}
      ${LLVM_INLINER_MODEL_PATH_DEFAULT}
      "models/gen-inline-oz-test-model.py"
      serve
      action
      InlinerSizeModel
      llvm::InlinerSizeModel
    )
  endif()

  if (LLVM_HAVE_TFLITE)
    list(APPEND MLLinkDeps
      tensorflow-lite::tensorflow-lite)
  endif()
endif()

# INTEL_CUSTOMIZATION
if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  include(Intel_BuildOptReportProto)

  add_definitions(-DGOOGLE_PROTOBUF_NO_RTTI)
  add_definitions(-DINTEL_ENABLE_PROTO_BIN_OPTRPT)

  set(LLVM_OPTIONAL_SOURCES ${LLVM_OPTIONAL_SOURCES} ${OPT_REPORT_PROTO_SRCS})

  add_llvm_library(LLVMIntel_OptReportProtoWriter
    ${OPT_REPORT_PROTO_SRCS}
    ${OPT_REPORT_PROTO_HDRS}
    PARTIAL_SOURCES_INTENDED

    LINK_LIBS
    protobuf::libprotobuf
    )
  target_include_directories(LLVMIntel_OptReportProtoWriter
      SYSTEM PRIVATE $<TARGET_PROPERTY:protobuf::libprotobuf,INCLUDE_DIRECTORIES>)
  target_include_directories(LLVMIntel_OptReportProtoWriter PRIVATE ${OPT_REPORT_PROTO_DIR})

endif()
# end INTEL_CUSTOMIZATION

add_llvm_component_library(LLVMAnalysis
  AliasAnalysis.cpp
  AliasAnalysisEvaluator.cpp
  AliasSetTracker.cpp
  Analysis.cpp
  AssumeBundleQueries.cpp
  AssumptionCache.cpp
  BasicAliasAnalysis.cpp
  BlockFrequencyInfo.cpp
  BlockFrequencyInfoImpl.cpp
  BranchProbabilityInfo.cpp
  CFG.cpp
  CFGPrinter.cpp
  CFGSCCPrinter.cpp
  CGSCCPassManager.cpp
  CallGraph.cpp
  CallGraphSCCPass.cpp
  CallPrinter.cpp
  CaptureTracking.cpp
  CmpInstAnalysis.cpp
  CostModel.cpp
  CodeMetrics.cpp
  ConstantFolding.cpp
  CycleAnalysis.cpp
  DDG.cpp
  DDGPrinter.cpp
  ConstraintSystem.cpp
  Delinearization.cpp
  DemandedBits.cpp
  DependenceAnalysis.cpp
  DependenceGraphBuilder.cpp
  DevelopmentModeInlineAdvisor.cpp
  DomPrinter.cpp
  DomTreeUpdater.cpp
  DominanceFrontier.cpp
  FunctionPropertiesAnalysis.cpp
  GlobalsModRef.cpp
  GuardUtils.cpp
  HeatUtils.cpp
  IRSimilarityIdentifier.cpp
  IVDescriptors.cpp
  IVUsers.cpp
  ImportedFunctionsInliningStatistics.cpp
  IndirectCallPromotionAnalysis.cpp
  InlineCost.cpp
  InlineAdvisor.cpp
  InlineOrder.cpp
  InlineSizeEstimatorAnalysis.cpp
  InstCount.cpp
  InstructionPrecedenceTracking.cpp
  InstructionSimplify.cpp
# INTEL_CUSTOMIZATION
  Intel_Andersens.cpp
  Intel_ArrayUseAnalysis.cpp
  Intel_DopeVectorAnalysis.cpp
  Intel_DopeVectorTypeAnalysis.cpp
  Intel_FPValueRange.cpp
  Intel_FPValueRangeAnalysis.cpp
# INTEL_FEATURE_SW_ADVANCED
  Intel_IPCloningAnalysis.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_InlineCost.cpp
# INTEL_FEATURE_SW_ADVANCED
  Intel_LangRules.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_OPAnalysisUtils.cpp
# INTEL_FEATURE_SW_ADVANCED
  Intel_PartialInlineAnalysis.cpp
#end INTEL_FEATURE_SW_ADVANCED
  Intel_SNode.cpp
  Intel_StdContainerAA.cpp
  Intel_OptVLS.cpp
  Intel_OptVLSClientUtils.cpp
  Intel_WP.cpp
  Intel_XmainOptLevelPass.cpp
  Intel_MemRefAnalysis.cpp
  Intel_InlineReport/Intel_InlineReport.cpp
  Intel_InlineReport/Intel_MDInlineReport.cpp
  Intel_InlineReport/Intel_InlineReportSetup.cpp
  Intel_InlineReport/Intel_InlineReportCommon.cpp
  Intel_InlineReport/Intel_InlineReportEmitter.cpp
  Intel_OptReport/Diag.cpp
  Intel_OptReport/OptReport.cpp
  Intel_OptReport/OptReportPrintUtils.cpp
  Intel_OptReport/OptReportOptionsPass.cpp
  Intel_OptReport/OptReportSupport.cpp
# INTEL_FEATURE_MARKERCOUNT
  Intel_MarkerCountInfo.cpp
# end INTEL_FEATURE_MARKERCOUNT
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  Directives.cpp
  VPO/Utils/IntrinsicUtils.cpp             # TODO: Move to lib/Analysis
  VPO/Utils/OpenMPDirectivesAndClauses.cpp # TODO: Move to lib/Analysis
# end INTEL_COLLAB
  InteractiveModelRunner.cpp
  Interval.cpp
  IntervalPartition.cpp
  LazyBranchProbabilityInfo.cpp
  LazyBlockFrequencyInfo.cpp
  LazyCallGraph.cpp
  LazyValueInfo.cpp
  Lint.cpp
  Loads.cpp
  Local.cpp
  LoopAccessAnalysis.cpp
  LoopAnalysisManager.cpp
  LoopCacheAnalysis.cpp
  LoopNestAnalysis.cpp
  LoopUnrollAnalyzer.cpp
  LoopInfo.cpp
  LoopPass.cpp
  MLInlineAdvisor.cpp
  MemDerefPrinter.cpp
  MemoryBuiltins.cpp
  MemoryDependenceAnalysis.cpp
  MemoryLocation.cpp
  MemoryProfileInfo.cpp
  MemorySSA.cpp
  MemorySSAUpdater.cpp
  ModelUnderTrainingRunner.cpp
  ModuleDebugInfoPrinter.cpp
  ModuleSummaryAnalysis.cpp
  MustExecute.cpp
  NoInferenceModelRunner.cpp
  ObjCARCAliasAnalysis.cpp
  ObjCARCAnalysisUtils.cpp
  ObjCARCInstKind.cpp
  OptimizationRemarkEmitter.cpp
  OverflowInstAnalysis.cpp
  PHITransAddr.cpp
  PhiValues.cpp
  PostDominators.cpp
  ProfileSummaryInfo.cpp
  PtrUseVisitor.cpp
  RegionInfo.cpp
  RegionPass.cpp
  RegionPrinter.cpp
  ReplayInlineAdvisor.cpp
  ScalarEvolution.cpp
  ScalarEvolutionAliasAnalysis.cpp
  ScalarEvolutionDivision.cpp
  ScalarEvolutionNormalization.cpp
  StackLifetime.cpp
  StackSafetyAnalysis.cpp
  StructuralHash.cpp
  SyntheticCountsUtils.cpp
  TFLiteUtils.cpp
  TargetLibraryInfo.cpp
  TargetTransformInfo.cpp
  TensorSpec.cpp
  Trace.cpp
  TrainingLogger.cpp
  TypeBasedAliasAnalysis.cpp
  TypeMetadataUtils.cpp
  UniformityAnalysis.cpp
  ScopedNoAliasAA.cpp
  ValueLattice.cpp
  ValueLatticeUtils.cpp
  ValueTracking.cpp
  VectorUtils.cpp
  VFABIDemangling.cpp
  ${GeneratedMLSources}

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/IPO

  DEPENDS
  intrinsics_gen
  ${MLDeps}
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB
  intel_svml_gen          #INTEL_CUSTOMIZATION -- TODO: VEC to COLLAB
  intel_svml_device_gen   #INTEL_CUSTOMIZATION
  intel_libmvec_gen       #INTEL_CUSTOMIZATION
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_ADVANCED
  LLVMDemangle
# end INTEL_FEATURE_SW_ADVANCED
# end INTEL_CUSTOMIZATION

  LINK_LIBS
  ${MLLinkDeps}
# INTEL_CUSTOMIZATION
# INTEL_FEATURE_SW_ADVANCED
  LLVMDemangle
# end INTEL_FEATURE_SW_ADVANCED
# end INTEL_CUSTOMIZATION

  LINK_COMPONENTS
  BinaryFormat
  Core
  Object
  ProfileData
  Support
  TargetParser
  )

# INTEL_CUSTOMIZATION
if(INTEL_ENABLE_PROTO_BIN_OPTRPT)
  target_link_libraries(LLVMAnalysis
    PRIVATE
    protobuf::libprotobuf
    LLVMIntel_OptReportProtoWriter
    )
  target_include_directories(LLVMAnalysis
      SYSTEM PRIVATE $<TARGET_PROPERTY:protobuf::libprotobuf,INCLUDE_DIRECTORIES>)
  target_include_directories(LLVMAnalysis PRIVATE ${OPT_REPORT_PROTO_DIR})
endif()
# end INTEL_CUSTOMIZATION

add_subdirectory(Intel_LoopAnalysis)  #INTEL_CUSTOMIZATION
# INTEL_COLLAB
add_subdirectory(VPO)
# end INTEL_COLLAB
