if (DEFINED LLVM_HAVE_TF_AOT OR DEFINED LLVM_HAVE_TF_API)
  if (DEFINED LLVM_HAVE_TF_AOT)
    include(TensorFlowCompile)
    tfcompile(models/inliner serve action InlinerSizeModel llvm::InlinerSizeModel)
    list(APPEND GeneratedMLSources
      $<TARGET_OBJECTS:tf_xla_runtime_objects>
      ${GENERATED_OBJS}
    )
  endif()

  if (DEFINED LLVM_HAVE_TF_API)
    LIST(APPEND MLLinkDeps ${tensorflow_c_api})
  endif()
endif()

add_llvm_component_library(LLVMAnalysis
  AliasAnalysis.cpp
  AliasAnalysisEvaluator.cpp
  AliasAnalysisSummary.cpp
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
  CFLAndersAliasAnalysis.cpp
  CFLSteensAliasAnalysis.cpp
  CGSCCPassManager.cpp
  CallGraph.cpp
  CallGraphSCCPass.cpp
  CallPrinter.cpp
  CaptureTracking.cpp
  CmpInstAnalysis.cpp
  CostModel.cpp
  CodeMetrics.cpp
  ConstantFolding.cpp
  DDG.cpp
  Delinearization.cpp
  DemandedBits.cpp
  DependenceAnalysis.cpp
  DependenceGraphBuilder.cpp
  DevelopmentModeInlineAdvisor.cpp
  DivergenceAnalysis.cpp
  DomPrinter.cpp
  DomTreeUpdater.cpp
  DominanceFrontier.cpp
  EHPersonalities.cpp
  FunctionPropertiesAnalysis.cpp
  GlobalsModRef.cpp
  GuardUtils.cpp
  HeatUtils.cpp
  IVDescriptors.cpp
  IVUsers.cpp
  IndirectCallPromotionAnalysis.cpp
  InlineCost.cpp
  InlineAdvisor.cpp
  InlineSizeEstimatorAnalysis.cpp
  InstCount.cpp
  InstructionPrecedenceTracking.cpp
  InstructionSimplify.cpp
# INTEL_CUSTOMIZATION
  Intel_Andersens.cpp
  Intel_ArrayUseAnalysis.cpp
  Intel_DopeVectorAnalysis.cpp
  Intel_IPCloningAnalysis.cpp
  Intel_PartialInlineAnalysis.cpp
  Intel_SNode.cpp
  Intel_StdContainerAA.cpp
  Intel_OptVLS.cpp
  Intel_OptVLSClientUtils.cpp
  Intel_VectorVariant.cpp
  Intel_WP.cpp
  Intel_XmainOptLevelPass.cpp
  Intel_MemRefAnalysis.cpp
  Intel_InlineReport/Intel_InlineReport.cpp
  Intel_InlineReport/Intel_MDInlineReport.cpp
  Intel_InlineReport/Intel_InlineReportSetup.cpp
  Intel_InlineReport/Intel_InlineReportCommon.cpp
  Intel_InlineReport/Intel_InlineReportEmitter.cpp
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  Directives.cpp
  VPO/Utils/IntrinsicUtils.cpp             # TODO: Move to lib/Analysis
  VPO/Utils/OpenMPDirectivesAndClauses.cpp # TODO: Move to lib/Analysis
# end INTEL_COLLAB
  Interval.cpp
  IntervalPartition.cpp
  LazyBranchProbabilityInfo.cpp
  LazyBlockFrequencyInfo.cpp
  LazyCallGraph.cpp
  LazyValueInfo.cpp
  LegacyDivergenceAnalysis.cpp
  Lint.cpp
  Loads.cpp
  LoopAccessAnalysis.cpp
  LoopAnalysisManager.cpp
  LoopCacheAnalysis.cpp
  LoopNestAnalysis.cpp
  LoopUnrollAnalyzer.cpp
  LoopInfo.cpp
  LoopPass.cpp
  MLInlineAdvisor.cpp
  MemDepPrinter.cpp
  MemDerefPrinter.cpp
  MemoryBuiltins.cpp
  MemoryDependenceAnalysis.cpp
  MemoryLocation.cpp
  MemorySSA.cpp
  MemorySSAUpdater.cpp
  ModuleDebugInfoPrinter.cpp
  ModuleSummaryAnalysis.cpp
  MustExecute.cpp
  ObjCARCAliasAnalysis.cpp
  ObjCARCAnalysisUtils.cpp
  ObjCARCInstKind.cpp
  OptimizationRemarkEmitter.cpp
  PHITransAddr.cpp
  PhiValues.cpp
  PostDominators.cpp
  ProfileSummaryInfo.cpp
  PtrUseVisitor.cpp
  RegionInfo.cpp
  RegionPass.cpp
  RegionPrinter.cpp
  ReleaseModeModelRunner.cpp
  ScalarEvolution.cpp
  ScalarEvolutionAliasAnalysis.cpp
  ScalarEvolutionDivision.cpp
  ScalarEvolutionNormalization.cpp
  StackLifetime.cpp
  StackSafetyAnalysis.cpp
  SyncDependenceAnalysis.cpp
  SyntheticCountsUtils.cpp
  TFUtils.cpp
  TargetLibraryInfo.cpp
  TargetTransformInfo.cpp
  Trace.cpp
  TypeBasedAliasAnalysis.cpp
  TypeMetadataUtils.cpp
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
# INTEL_COLLAB
  directives_gen
# end INTEL_COLLAB
  intel_svml_gen          #INTEL_CUSTOMIZATION -- TODO: VEC to COLLAB
  intel_libmvec_gen       #INTEL_CUSTOMIZATION

  LINK_LIBS
  ${MLLinkDeps}
  )

add_subdirectory(Intel_LoopAnalysis)  #INTEL_CUSTOMIZATION
add_subdirectory(Intel_OptReport)     #INTEL_CUSTOMIZATION
# INTEL_COLLAB
add_subdirectory(VPO)
# end INTEL_COLLAB
