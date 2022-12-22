; INTEL_CUSTOMIZATION
; RUN: opt -disable-verify -disable-output -verify-cfg-preserved=0 \
; RUN:     -debug-pass-manager  -passes='default<O2>' \
; RUN:     -paropt=31 -vecopt=true -S %s 2>&1 | FileCheck %s

;             Running pass: XmainOptLevelAnalysisInit ;INTEL
;             Running analysis: XmainOptLevelAnalysis ;INTEL
;             Running pass: Annotation2MetadataPass
;             Running pass: ForceFunctionAttrsPass
;             Running pass: InferFunctionAttrsPass
;             Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;             Running analysis: TargetLibraryAnalysis
; CHECK:      Running pass: RequireAnalysisPass<llvm::VPOParoptConfigAnalysis, {{.*Module.*}}> ;INTEL
; CHECK-NEXT: Running analysis: VPOParoptConfigAnalysis                                        ;INTEL
; CHECK-NEXT: Running pass: InlineReportSetupPass
; CHECK-NEXT: Running pass: InlineListsPass
; CHECK-NEXT: Running pass: CoroEarlyPass
; CHECK-NEXT: Running pass: LowerSubscriptIntrinsicPass
; CHECK-NEXT: Running pass: DopeVectorConstPropPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: VPOParoptApplyConfigPass
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-DAG:  Running analysis: TargetLibraryAnalysis
; CHECK-DAG:  Running analysis: AssumptionAnalysis
; CHECK-DAG:  Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running analysis: ScopedNoAliasAA
; CHECK-NEXT: Running analysis: TypeBasedAA
; CHECK-NEXT: Running analysis: StdContainerAA
; CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running pass: VPOParoptLoopTransformPass
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPOParoptLoopCollapsePass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: VPOParoptPreparePass
; CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
; CHECK-NEXT: Running pass: LowerExpectIntrinsicPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: EarlyCSEPass
; CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAnalysis
; CHECK-NEXT: Invalidating analysis: BasicAA
; CHECK-NEXT: Invalidating analysis: AAManager
; CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running pass: IPSCCPPass
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: CalledValuePropagationPass
; CHECK-NEXT: Running pass: GlobalOptPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running pass: JumpThreadingPass
; CHECK-NEXT: Running analysis: LazyValueAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAnalysis
; CHECK-NEXT: Invalidating analysis: BasicAA
; CHECK-NEXT: Invalidating analysis: AAManager
; CHECK-NEXT: Invalidating analysis: LazyValueAnalysis
; CHECK-NEXT: Invalidating analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: ModuleInlinerWrapperPass
; CHECK-NEXT: Running analysis: InlineAdvisorAnalysi
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::GlobalsAA{{.*}}>
; CHECK-NEXT: Running analysis: GlobalsAA
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: InvalidateAnalysisPass<llvm::AAManager>
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis{{.*}}>
; CHECK-NEXT: Running analysis: ProfileSummaryAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running analysis: LazyCallGraphAnalysis
; CHECK-NEXT: Running analysis: FunctionAnalysisManagerCGSCCProxy
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*SCC.*}}>
; CHECK-NEXT: Running pass: InlinerPass
; CHECK-NEXT: Running pass: InlinerPass
; CHECK-NEXT: Invalidating analysis: InlineAdvisorAnalysis
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: VPORestoreOperandsPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: VPOParoptSharedPrivatizationPass
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass
; CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAnalysis
; CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Invalidating analysis: BasicAA
; CHECK-NEXT: Invalidating analysis: AAManager
; CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: LazyCallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running pass: VPOParoptPass
; CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running pass: VPOCFGSimplifyPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPOParoptGuardMemoryMotionPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPORenameOperandsPass
; CHECK-NEXT: Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running pass: OpenMPOptPass
; CHECK-NEXT: Running pass: GlobalDCEPass
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*Function.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running pass: ModuleInlinerWrapperPass
; CHECK-NEXT: Running analysis: InlineAdvisorAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*Function.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::GlobalsAA, llvm::Module>
; CHECK-NEXT: Running pass: InvalidateAnalysisPass<llvm::AAManager>
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis{{.*}}>
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running analysis: LazyCallGraphAnalysis
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running analysis: FunctionAnalysisManagerCGSCCProxy
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*SCC.*}}>
; CHECK-NEXT: Running pass: DevirtSCCRepeatedPass
; CHECK-NEXT: Running pass: InlinerPass
; CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-NEXT: Running pass: InlinerPass
; CHECK-NEXT: Running pass: ArgumentPromotionPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-DAG:  Running analysis: DominatorTreeAnalysis
; CHECK-DAG:  Running analysis: AssumptionAnalysis
; CHECK-DAG:  Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running pass: PostOrderFunctionAttrsPass
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running analysis: ScopedNoAliasAA
; CHECK-NEXT: Running analysis: TypeBasedAA
; CHECK-NEXT: Running analysis: StdContainerAA
; CHECK-NEXT: Running pass: OpenMPOptCGSCCPass
; CHECK-NEXT: Running pass: TbaaMDPropagationPass
; CHECK-NEXT: Running pass: RequireAnalysisPass<{{.*OptReportOptionsAnalysis.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: EarlyCSEPass
; CHECK-NEXT: Running analysis: MemorySSAAnalysis
; CHECK-NEXT: Running pass: SpeculativeExecutionPass
; CHECK-NEXT: Running pass: JumpThreadingPass
; CHECK-NEXT: Running analysis: LazyValueAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: CorrelatedValuePropagationPass
; CHECK-NEXT: Invalidating analysis: LazyValueAnalysis
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: LibCallsShrinkWrapPass
; CHECK-NEXT: Running pass: TailCallElimPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: ReassociatePass
; CHECK-NEXT: Running pass: RequireAnalysisPass<{{.*OptimizationRemarkEmitterAnalysis.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*LoopAnalysisManager.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running pass: LoopInstSimplifyPass
; CHECK-NEXT: Running pass: LoopSimplifyCFGPass
; CHECK-NEXT: Running pass: LICMPass
; CHECK-NEXT: Running pass: LoopRotatePass
; CHECK-NEXT: Running pass: LICMPass
; CHECK-NEXT: Running pass: SimpleLoopUnswitchPass
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*FunctionAnalysisManager.*}}, {{.*Loop.*}}>
; CHECK-NEXT: Running pass: LoopFlattenPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running pass: LoopIdiomRecognizePass
; CHECK-NEXT: Running pass: IndVarSimplifyPass
; CHECK-NEXT: Running pass: LoopDeletionPass
; CHECK-NEXT: Running pass: LoopFullUnrollPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: VectorCombinePass
; CHECK-NEXT: Running pass: MergedLoadStoreMotionPass
; CHECK-NEXT: Running pass: GVNPass
; CHECK-NEXT: Running analysis: MemoryDependenceAnalysis
; CHECK-NEXT: Running analysis: PhiValuesAnalysis
; CHECK-NEXT: Running pass: SCCPPass
; CHECK-NEXT: Running pass: BDCEPass
; CHECK-NEXT: Running analysis: DemandedBitsAnalysis
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: JumpThreadingPass
; CHECK-NEXT: Running analysis: LazyValueAnalysis
; CHECK-NEXT: Running pass: CorrelatedValuePropagationPass
; CHECK-NEXT: Invalidating analysis: LazyValueAnalysis
; CHECK-NEXT: Running pass: ADCEPass
; CHECK-NEXT: Running pass: MemCpyOptPass
; CHECK-NEXT: Running pass: DSEPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running pass: LICMPass
; CHECK-NEXT: Running pass: CoroElidePass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: InstCombinePass
; CHECK-NEXT: Running pass: TransformSinAndCosCallsPass
; CHECK-NEXT: Clearing all analysis results for: <possibly invalidated loop>
; CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: BasicAA
; CHECK-NEXT: Invalidating analysis: AAManager
; CHECK-NEXT: Invalidating analysis: MemorySSAAnalysis
; CHECK-NEXT: Invalidating analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAnalysis
; CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*LoopAnalysisManager.*}}, {{.*Function.*}}>
; CHECK-NEXT: Invalidating analysis: PhiValuesAnalysis
; CHECK-NEXT: Invalidating analysis: MemoryDependenceAnalysis
; CHECK-NEXT: Invalidating analysis: DemandedBitsAnalysis
; CHECK-NEXT: Running analysis: ShouldNotRunFunctionPassesAnalysis
; CHECK-NEXT: Running pass: CoroSplitPass
; CHECK-NEXT: Running pass: InvalidateAnalysisPass<{{.*ShouldNotRunFunctionPassesAnalysis.*}}>
; CHECK-NEXT: Invalidating analysis: ShouldNotRunFunctionPassesAnalysis
; CHECK-NEXT: Invalidating analysis: InlineAdvisorAnalysis
; CHECK-NEXT: Running pass: IPSCCPPass
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: DeadArgumentEliminationPass
; CHECK-NEXT: Running pass: CoroCleanupPass
; CHECK-NEXT: Running pass: GlobalOptPass
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running pass: GlobalDCEPass
; CHECK-NEXT: Running pass: StdContainerOptPass
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running pass: CleanupFakeLoadsPass
; CHECK-NEXT: Running pass: EliminateAvailableExternallyPass
; CHECK-NEXT: Running pass: ReversePostOrderFunctionAttrsPass
; CHECK-NEXT: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::AndersensAA, {{.*Module.*}}>
; CHECK-NEXT: Running analysis: AndersensAA
; CHECK-NEXT: Running pass: NonLTOGlobalOptPass
; CHECK-NEXT: Running pass: PromotePass
; CHECK-NEXT: Running pass: ADCEPass
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running pass: RecomputeGlobalsAAPass
; CHECK-NEXT: Running pass: Float2IntPass
; CHECK-NEXT: Running pass: LowerConstantIntrinsicsPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*LoopAnalysisManager.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running pass: LoopRotatePass
; CHECK-NEXT: Running pass: LoopDeletionPass
; CHECK-NEXT: Running pass: VecClonePass
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*FunctionAnalysisManager.*}}, {{.*Module.*}}>
; CHECK-NEXT: Invalidating analysis: LazyCallGraphAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*CGSCCAnalysisManager.*}}, {{.*Module.*}}>
; CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*FunctionAnalysisManager.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running pass: EarlyCSEPass
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running pass: AlignmentFromAssumptionsPass
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: VPORestoreOperandsPass
; CHECK-NEXT: Running pass: VPlanPragmaOmpSimdIfPass
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running pass: LowerSwitchPass
; CHECK-NEXT: Running analysis: LazyValueAnalysis
; CHECK-NEXT: Running pass: LCSSAPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPlanPragmaOmpOrderedSimdExtractPass
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*ModuleAnalysisManager.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running analysis: ScopedNoAliasAA
; CHECK-NEXT: Running analysis: TypeBasedAA
; CHECK-NEXT: Running analysis: StdContainerAA
; CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*FunctionAnalysisManager.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*FunctionAnalysisManager.*}}, {{.*Module.*}}>
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; CHECK-NEXT: Running pass: vpo::VPlanDriverPass
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-DAG: Running analysis: TargetLibraryAnalysis
; CHECK-DAG: Running analysis: AssumptionAnalysis
; CHECK-DAG: Running analysis: TargetIRAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*ModuleAnalysisManager.*}}, {{.*Function.*}}>
; CHECK-NEXT: Running analysis: ScopedNoAliasAA
; CHECK-NEXT: Running analysis: TypeBasedAA
; CHECK-NEXT: Running analysis: StdContainerAA
; CHECK-NEXT: Running analysis: DemandedBitsAnalysis
; CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: BlockFrequencyAnalysis
; CHECK-NEXT: Running analysis: BranchProbabilityAnalysis
; CHECK-NEXT: Running analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Running analysis: LoopAccessAnalysis
; CHECK-NEXT: Running pass: MathLibraryFunctionsReplacementPass
; CHECK-NEXT: Running pass: AlwaysInlinerPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPODirectiveCleanupPass
; CHECK-NEXT: Running pass: SROAPass
; CHECK-NEXT: Running pass: SimplifyCFGPass
; CHECK-NEXT: Running pass: VPORestoreOperandsPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPOParoptSharedPrivatizationPass
; CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass
; CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAnalysis
; CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Invalidating analysis: BasicAA
; CHECK-NEXT: Invalidating analysis: AAManager
; CHECK-NEXT: Invalidating analysis: DemandedBitsAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
; CHECK-NEXT: Invalidating analysis: PostDominatorTreeAnalysis
; CHECK-NEXT: Invalidating analysis: BranchProbabilityAnalysis
; CHECK-NEXT: Invalidating analysis: BlockFrequencyAnalysis
; CHECK-NEXT: Invalidating analysis: LoopAccessAnalysis
; CHECK-NEXT: Running pass: LoopSimplifyPass
; CHECK-NEXT: Running analysis: LoopAnalysis
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-NEXT: Running pass: VPOParoptPass
; CHECK-NEXT: Running analysis: WRegionInfoAnalysis
; CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
; CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
; CHECK-NEXT: Running analysis: AAManager
; CHECK-NEXT: Running analysis: BasicAA
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPODirectiveCleanupPass
; CHECK-NEXT: Running pass: VPOCFGSimplifyPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPOParoptGuardMemoryMotionPass
; CHECK-NEXT: Running pass: VPOCFGRestructuringPass
; CHECK-NEXT: Running pass: VPORenameOperandsPass
;             Running pass: AlwaysInlinerPass
;             Running pass: GlobalDCEPass
;             Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, {{.*Module.*}}>
;             Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, {{.*Module.*}}>
;             Running pass: TightLoopEmitterPass
;             Running analysis: OptReportOptionsAnalysis
;             Running analysis: LoopAnalysis
;             Running analysis: DominatorTreeAnalysis
;             Running pass: LoopDistributePass
;             Running analysis: ScalarEvolutionAnalysis
;             Running analysis: TargetLibraryAnalysis
;             Running analysis: AssumptionAnalysis
;             Running analysis: TargetIRAnalysis
;             Running analysis: OptimizationRemarkEmitterAnalysis
;             Running analysis: LoopAccessAnalysis
;             Running analysis: AAManager
;             Running analysis: BasicAA
;             Running analysis: XmainOptLevelAnalysis
;             Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, {{.*Function.*}}>
;             Running analysis: ScopedNoAliasAA
;             Running analysis: TypeBasedAA
;             Running analysis: StdContainerAA
;             Running pass: InjectTLIMappings
;             Running pass: LoopLoadEliminationPass
; CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
; CHECK:      Running pass: InstCombinePass
;             Running pass: SimplifyCFGPass
;             Running pass: SLPVectorizerPass
;             Running analysis: DemandedBitsAnalysis
;             Running pass: LoadCoalescingPass
;             Running pass: SROAPass
;             Running pass: VectorCombinePass
;             Running pass: EarlyCSEPass ;INTEL
; CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
; CHECK:      Running pass: InstCombinePass
;             Running pass: LoopUnrollPass
;             Running pass: SROAPass
;             Running pass: WarnMissedTransformationsPass
; CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
; CHECK:      Running pass: InstCombinePass
;             Running pass: SROAPass
;             Running pass: WarnMissedTransformationsPass
;             Running pass: VPOCFGRestructuringPass
;             Running pass: InstCombinePass
;             Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
;             Running pass: LoopSimplifyPass
;             Running pass: LCSSAPass
;             Running analysis: MemorySSAAnalysis
;             Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;             Running pass: LICMPass
;             Running pass: AlignmentFromAssumptionsPass
;             Running pass: LoopSinkPass
;             Running analysis: BlockFrequencyAnalysis
;             Running analysis: BranchProbabilityAnalysis
;             Running analysis: PostDominatorTreeAnalysis
;             Running pass: InstSimplifyPass
;             Running pass: DivRemPairsPass
;             Running pass: TailCallElimPass
;             Running pass: SimplifyCFGPass
;             Invalidating analysis: DominatorTreeAnalysis
;             Invalidating analysis: LoopAnalysis
;             Invalidating analysis: ScalarEvolutionAnalysis
;             Invalidating analysis: BasicAA
;             Invalidating analysis: AAManager
;             Invalidating analysis: LoopAccessAnalysis
;             Invalidating analysis: DemandedBitsAnalysis
;             Invalidating analysis: MemorySSAAnalysis
;             Invalidating analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;             Invalidating analysis: PostDominatorTreeAnalysis
;             Invalidating analysis: BranchProbabilityAnalysis
;             Invalidating analysis: BlockFrequencyAnalysis
;             Running pass: GlobalDCEPass
;             Running pass: ConstantMergePass
;             Running pass: CGProfilePass
;             Running analysis: BlockFrequencyAnalysis
;             Running analysis: BranchProbabilityAnalysis
;             Running analysis: LoopAnalysis
;             Running analysis: DominatorTreeAnalysis
;             Running analysis: PostDominatorTreeAnalysis
;             Running pass: RelLookupTableConverterPass
;             Running pass: InlineReportEmitterPass ;INTEL
;             Running pass: AnnotationRemarksPass

; The IR below was taken from new-pm-O0-defaults.ll

declare void @bar() local_unnamed_addr

define void @foo(i32 %n) local_unnamed_addr {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  tail call void @bar()
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}
; end INTEL_CUSTOMIZATION
