add_llvm_library(LLVMAnalysis
  AliasAnalysis.cpp
  AliasAnalysisEvaluator.cpp
  AliasAnalysisSummary.cpp
  AliasSetTracker.cpp
  Analysis.cpp
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
  Delinearization.cpp
  DemandedBits.cpp
  DependenceAnalysis.cpp
  DomPrinter.cpp
  DominanceFrontier.cpp
  EHPersonalities.cpp
  GlobalsModRef.cpp
  GuardUtils.cpp
  IVDescriptors.cpp
  IVUsers.cpp
  IndirectCallPromotionAnalysis.cpp
  InlineCost.cpp
  InstCount.cpp
  InstructionPrecedenceTracking.cpp
  InstructionSimplify.cpp
# INTEL_CUSTOMIZATION
  Intel_AggInline.cpp
  Intel_Andersens.cpp
  Intel_IPCloningAnalysis.cpp
  Intel_SNode.cpp
  Intel_StdContainerAA.cpp
  Intel_OptVLS.cpp
  Intel_OptVLSClientUtils.cpp
  Intel_VectorVariant.cpp
  Intel_WP.cpp
  Intel_XmainOptLevelPass.cpp
# end INTEL_CUSTOMIZATION
# INTEL_COLLAB
  Intel_Directives.cpp
  Intel_VPO/Utils/IntrinsicUtils.cpp             # TODO: Move to lib/Analysis
  Intel_VPO/Utils/OpenMPDirectivesAndClauses.cpp # TODO: Move to lib/Analysis
# end INTEL_COLLAB
  Interval.cpp
  IntervalPartition.cpp
  IteratedDominanceFrontier.cpp
  LazyBranchProbabilityInfo.cpp
  LazyBlockFrequencyInfo.cpp
  LazyCallGraph.cpp
  LazyValueInfo.cpp
  LegacyDivergenceAnalysis.cpp
  Lint.cpp
  Loads.cpp
  LoopAccessAnalysis.cpp
  LoopAnalysisManager.cpp
  LoopUnrollAnalyzer.cpp
  LoopInfo.cpp
  LoopPass.cpp
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
  OrderedBasicBlock.cpp
  OrderedInstructions.cpp
  PHITransAddr.cpp
  PhiValues.cpp
  PostDominators.cpp
  ProfileSummaryInfo.cpp
  PtrUseVisitor.cpp
  RegionInfo.cpp
  RegionPass.cpp
  RegionPrinter.cpp
  ScalarEvolution.cpp
  ScalarEvolutionAliasAnalysis.cpp
  ScalarEvolutionExpander.cpp
  ScalarEvolutionNormalization.cpp
  SyntheticCountsUtils.cpp
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

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Analysis

  DEPENDS
  intrinsics_gen
# INTEL_COLLAB
  intel_directives_gen
# end INTEL_COLLAB
  intel_svml_gen          #INTEL_CUSTOMIZATION -- TODO: VEC to COLLAB
  intel_libmvec_gen       #INTEL_CUSTOMIZATION
  )

add_subdirectory(Intel_LoopAnalysis)  #INTEL_CUSTOMIZATION
add_subdirectory(Intel_OptReport)     #INTEL_CUSTOMIZATION
# INTEL_COLLAB
add_subdirectory(Intel_VPO)
# end INTEL_COLLAB
