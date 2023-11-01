; INTEL_CUSTOMIZATION
; RUN: opt -disable-verify -disable-output -verify-analysis-invalidation=0 \
; RUN:     -debug-pass-manager  -passes='default<O2>' \
; RUN:     -paropt=31 -vecopt=true -loopopt=1 -S %s 2>&1 | FileCheck %s

;            Running pass: XmainOptLevelAnalysisInit  ;INTEL
;            Running analysis: XmainOptLevelAnalysis  ;INTEL
;            Running pass: Annotation2MetadataPass
;            Running pass: ForceFunctionAttrsPass
;            Running pass: InferFunctionAttrsPass
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running analysis: TargetLibraryAnalysis
;CHECK:      Running pass: RequireAnalysisPass<llvm::VPOParoptConfigAnalysis{{.*Module.*}}> on [module] ;INTEL
;CHECK-NEXT: Running analysis: VPOParoptConfigAnalysis
;            Running pass: InlineReportSetupPass  ;INTEL
;            Running pass: InlineListsPass        ;INTEL
;            Running pass: CoroEarlyPass          ;INTEL
;CHECK:      Running pass: LoopOptMarkerPass
;CHECK-NEXT: Running pass: DopeVectorConstPropPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis
;CHECK-NEXT: Running analysis: LoopAnalysis
;CHECK-NEXT: Running pass: VPOParoptApplyConfigPass
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
;CHECK-DAG: Running analysis: TargetLibraryAnalysis
;CHECK-DAG: Running analysis: AssumptionAnalysis
;CHECK-DAG: Running analysis: TargetIRAnalysis
;CHECK-NEXT: Running analysis: AAManager
;CHECK-NEXT: Running analysis: BasicAA
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*ModuleAnalysisManager.*}}, {{.*Function.*}}>
;CHECK-NEXT: Running analysis: ScopedNoAliasAA
;CHECK-NEXT: Running analysis: TypeBasedAA
;CHECK-NEXT: Running analysis: StdContainerAA
;CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
;CHECK-NEXT: Running pass: VPOParoptLoopTransformPass
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: VPOParoptLoopCollapsePass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: LoopSimplifyPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: VPOParoptGuardMemoryMotionPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: VPOParoptPreparePass
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
;            Running pass: LowerExpectIntrinsicPass
;            Running pass: SimplifyCFGPass
;            Running pass: SROAPass
;            Running pass: EarlyCSEPass
;            Invalidating analysis: DominatorTreeAnalysis
;            Invalidating analysis: LoopAnalysis
;            Invalidating analysis: BasicAA
;            Invalidating analysis: AAManager
;            Invalidating analysis: ScalarEvolutionAnalysis
;            Invalidating analysis: WRegionCollectionAnalysis
;            Invalidating analysis: WRegionInfoAnalysis
;            Running pass: IPSCCPPass
;            Running analysis: DominatorTreeAnalysis
;            Running pass: CalledValuePropagationPass
;            Running pass: GlobalOptPass
;            Running pass: PromotePass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;            Running analysis: LoopAnalysis
;CHECK:      Running pass: InstCombinePass
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running pass: JumpThreadingPass
;            Running analysis: LazyValueAnalysis
;            Running analysis: PostDominatorTreeAnalysis
;            Running pass: SimplifyCFGPass
;            Invalidating analysis: DominatorTreeAnalysis
;            Invalidating analysis: LoopAnalysis
;            Invalidating analysis: BasicAA
;            Invalidating analysis: AAManager
;            Invalidating analysis: LazyValueAnalysis
;            Invalidating analysis: PostDominatorTreeAnalysis
;CHECK:      Running pass: ModuleInlinerWrapperPass
;CHECK-NEXT: Running analysis: InlineAdvisorAnalysis
;CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::GlobalsAA{{.*}}>
;CHECK-NEXT: Running analysis: GlobalsAA
;CHECK-NEXT: Running analysis: CallGraphAnalysis
;CHECK-NEXT: Running pass: InvalidateAnalysisPass<llvm::AAManager>
;CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis{{.*}}>
;CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}>
;CHECK-NEXT: Running analysis: LazyCallGraphAnalysis
;CHECK-NEXT: Running analysis: FunctionAnalysisManagerCGSCCProxy
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*SCC.*}}>
;CHECK-NEXT: Running pass: InlinerPass
;CHECK-NEXT: Invalidating analysis: InlineAdvisorAnalysis
;CHECK-NEXT: Running pass: SROAPass
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis
;CHECK-NEXT: Running pass: SimplifyCFGPass
;CHECK-NEXT: Running pass: VPORestoreOperandsPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running analysis: LoopAnalysis
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptSharedPrivatizationPass
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
;CHECK-NEXT: Running analysis: AAManager
;CHECK-NEXT: Running analysis: BasicAA
;CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis
;CHECK-NEXT: Invalidating analysis: LoopAnalysis
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis
;CHECK-NEXT: Invalidating analysis: BasicAA
;CHECK-NEXT: Invalidating analysis: AAManager
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: LoopSimplifyPass
;CHECK-NEXT: Running analysis: LoopAnalysis
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis
;CHECK-NEXT: Invalidating analysis: CallGraphAnalysis
;CHECK-NEXT: Invalidating analysis: LazyCallGraphAnalysis
;CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}>
;CHECK-NEXT: Running pass: VPOParoptPass
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis
;CHECK-NEXT: Running analysis: AAManager
;CHECK-NEXT: Running analysis: BasicAA
;CHECK-NEXT: Running pass: VPOCFGSimplifyPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: VPOParoptGuardMemoryMotionPass
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass
;CHECK-NEXT: Running pass: VPORenameOperandsPass
;CHECK-NEXT: Running pass: AlwaysInlinerPass
;CHECK-NEXT: Running pass: OpenMPOptPass
;CHECK-NEXT: Running pass: GlobalDCEPass
;CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*Function.*}}, {{.*Module.*}}>
;            Running pass: ModuleInlinerWrapperPass
;            Running analysis: InlineAdvisorAnalysis
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running pass: RequireAnalysisPass<llvm::GlobalsAA, llvm::Module>
;            Running pass: InvalidateAnalysisPass<llvm::AAManager>
;            Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis, llvm::Module>
;            Running analysis: InnerAnalysisManagerProxy<llvm::CGSCCAnalysisManager, llvm::Module>
;            Running analysis: LazyCallGraphAnalysis
;            Running analysis: TargetLibraryAnalysis
;            Running analysis: FunctionAnalysisManagerCGSCCProxy
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, LazyCallGraph::SCC, llvm::LazyCallGraph &>
;            Running pass: DevirtSCCRepeatedPass
;            Running pass: InlinerPass
;            Running analysis: OptimizationRemarkEmitterAnalysis
;            Running pass: InlinerPass
;            Running pass: ArgumentPromotionPass
;            Running pass: SROAPass
;            Running analysis: DominatorTreeAnalysis
;            Running analysis: AssumptionAnalysis
;            Running analysis: TargetIRAnalysis
;            Running pass: PostOrderFunctionAttrsPass
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running analysis: XmainOptLevelAnalysis
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function>
;            Running analysis: ScopedNoAliasAA
;            Running analysis: TypeBasedAA
;            Running analysis: StdContainerAA
;            Running pass: OpenMPOptCGSCCPass
;            Running pass: TbaaMDPropagationPass
;            Running pass: RequireAnalysisPass<llvm::OptReportOptionsAnalysis, llvm::Function>
;            Running analysis: OptReportOptionsAnalysis
;            Running pass: SROAPass
;            Running pass: EarlyCSEPass
;            Running analysis: MemorySSAAnalysis
;            Running pass: SpeculativeExecutionPass
;            Running pass: JumpThreadingPass
;            Running analysis: LazyValueAnalysis
;            Running analysis: PostDominatorTreeAnalysis
;            Running pass: CorrelatedValuePropagationPass
;            Invalidating analysis: LazyValueAnalysis
;            Running pass: SimplifyCFGPass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;            Running analysis: LoopAnalysis
;CHECK:      Running pass: InstCombinePass
;            Running pass: LibCallsShrinkWrapPass
;            Running pass: TailCallElimPass
;            Running pass: SimplifyCFGPass
;            Running pass: ReassociatePass
;            Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
;            Running pass: LoopSimplifyPass
;            Running pass: LCSSAPass
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Running pass: LoopInstSimplifyPass
;            Running pass: LoopSimplifyCFGPass
;            Running pass: LICMPass
;            Running pass: LoopRotatePass
;            Running pass: LICMPass
;            Running pass: SimpleLoopUnswitchPass
;            Running analysis: OuterAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Loop, llvm::LoopStandardAnalysisResults &>
;            Running pass: SimplifyCFGPass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;CHECK:      Running pass: InstCombinePass
;            Running pass: LoopSimplifyPass
;            Running pass: LCSSAPass
;            Running pass: LoopIdiomRecognizePass
;            Running pass: IndVarSimplifyPass
;            Running pass: LoopDeletionPass
;            Running pass: SROAPass
;            Running pass: VectorCombinePass
;            Running pass: MergedLoadStoreMotionPass
;            Running pass: GVNPass
;            Running analysis: MemoryDependenceAnalysis
;            Running pass: SCCPPass
;            Running pass: BDCEPass
;            Running analysis: DemandedBitsAnalysis
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;CHECK:      Running pass: InstCombinePass
;            Running pass: JumpThreadingPass
;            Running analysis: LazyValueAnalysis
;            Running pass: CorrelatedValuePropagationPass
;            Invalidating analysis: LazyValueAnalysis
;            Running pass: ADCEPass
;            Running pass: MemCpyOptPass
;            Running pass: DSEPass
;            Running pass: LoopSimplifyPass
;            Running pass: LCSSAPass
;            Running pass: LICMPass
;            Running pass: CoroElidePass
;            Running pass: SimplifyCFGPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: InstCombinePass
;CHECK:      Running pass: TransformSinAndCosCallsPass
;            Clearing all analysis results for: <possibly invalidated loop>
;            Invalidating analysis: DominatorTreeAnalysis
;            Invalidating analysis: BasicAA
;            Invalidating analysis: AAManager
;            Invalidating analysis: MemorySSAAnalysis
;            Invalidating analysis: PostDominatorTreeAnalysis
;            Invalidating analysis: LoopAnalysis
;            Invalidating analysis: ScalarEvolutionAnalysis
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Invalidating analysis: MemoryDependenceAnalysis
;            Invalidating analysis: DemandedBitsAnalysis
;            Running analysis: ShouldNotRunFunctionPassesAnalysis
;            Running pass: CoroSplitPass
;            Running pass: InvalidateAnalysisPass<llvm::ShouldNotRunFunctionPassesAnalysis>
;            Invalidating analysis: ShouldNotRunFunctionPassesAnalysis
;            Invalidating analysis: InlineAdvisorAnalysis
;            Running pass: IPSCCPPass
;            Running analysis: DominatorTreeAnalysis
;            Running pass: DeadArgumentEliminationPass
;            Running pass: CoroCleanupPass
;            Running pass: GlobalOptPass
;            Running analysis: TargetLibraryAnalysis
;            Running pass: GlobalDCEPass
;            Running pass: StdContainerOptPass
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running pass: CleanupFakeLoadsPass
;            Running pass: EliminateAvailableExternallyPass
;            Running pass: ReversePostOrderFunctionAttrsPass
;            Running analysis: CallGraphAnalysis
;            Running pass: RequireAnalysisPass<llvm::AndersensAA, llvm::Module>
;            Running analysis: AndersensAA
;            Running pass: NonLTOGlobalOptPass
;            Running pass: PromotePass
;            Running pass: ADCEPass
;            Running analysis: PostDominatorTreeAnalysis
;            Running pass: RecomputeGlobalsAAPass
;            Running pass: Float2IntPass
;            Running pass: LowerConstantIntrinsicsPass
;            Running pass: LoopSimplifyPass
;            Running analysis: LoopAnalysis
;            Running pass: LCSSAPass
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Running pass: LoopRotatePass
;            Running pass: LoopDeletionPass
;CHECK:      Running pass: VecClonePass
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Invalidating analysis: LazyCallGraphAnalysis
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::CGSCCAnalysisManager, llvm::Module>
;            Invalidating analysis: CallGraphAnalysis
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running pass: EarlyCSEPass
;            Running analysis: TargetLibraryAnalysis
;            Running analysis: TargetIRAnalysis
;            Running analysis: DominatorTreeAnalysis
;CHECK:      Running analysis: AssumptionAnalysis
;CHECK:      Running pass: AlignmentFromAssumptionsPass
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: LoopAnalysis
;CHECK:      Running pass: VPORestoreOperandsPass
;CHECK:      Running pass: VPlanPragmaOmpSimdIfPass
;            Running pass: LoopSimplifyPass
;            Running pass: LCSSAPass
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running analysis: XmainOptLevelAnalysis
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function>
;            Running analysis: ScopedNoAliasAA
;            Running analysis: TypeBasedAA
;            Running analysis: StdContainerAA
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Running pass: LoopSimplifyCFGPass
;            Running pass: LCSSAPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPlanPragmaOmpOrderedSimdExtractPass
;            Running analysis: WRegionInfoAnalysis
;            Running analysis: WRegionCollectionAnalysis
;            Running analysis: OptimizationRemarkEmitterAnalysis
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;CHECK:      Running pass: VPlanPragmaOmpSimdIfPass
;            Running analysis: DominatorTreeAnalysis
;            Running analysis: LoopAnalysis
;            <Loopopt passes>
;CHECK:      Skipping pass: HIRVecDirInsertPass
;CHECK:      Skipping pass vpo::VPlanDriverHIRPass
;            <Loopopt passes>
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPORenameOperandsPass
;            Running analysis: WRegionInfoAnalysis
;            Running analysis: WRegionCollectionAnalysis
;            Running analysis: OptimizationRemarkEmitterAnalysis
;            Running pass: SimplifyCFGPass
;            Running pass: LowerSubscriptIntrinsicPass
;            Running pass: SROAPass
;            Running pass: GVNPass
;            Running analysis: MemoryDependenceAnalysis
;            Running pass: SROAPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: InstCombinePass
;            Running pass: LoopCarriedCSEPass
;            Running pass: DSEPass
;            Running analysis: MemorySSAAnalysis
;CHECK:      Running pass: VPORestoreOperandsPass
;            Running pass: LoopSimplifyPass
;            Running pass: LowerSwitchPass
;            Running analysis: LazyValueAnalysis
;            Running pass: LCSSAPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPlanPragmaOmpOrderedSimdExtractPass
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;CHECK:      Running pass: VPOCFGRestructuringPass
;            Running analysis: DominatorTreeAnalysis
;            Running analysis: LoopAnalysis
;CHECK:      Running pass: MathLibraryFunctionsReplacementPass
;CHECK:      Running pass: vpo::VPlanDriverLLVMPass
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: TargetLibraryAnalysis
;            Running analysis: AssumptionAnalysis
;            Running analysis: TargetIRAnalysis
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running analysis: XmainOptLevelAnalysis
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function>
;            Running analysis: ScopedNoAliasAA
;            Running analysis: TypeBasedAA
;            Running analysis: StdContainerAA
;            Running analysis: DemandedBitsAnalysis
;            Running analysis: OptimizationRemarkEmitterAnalysis
;            Running analysis: OptReportOptionsAnalysis
;            Running analysis: WRegionInfoAnalysis
;            Running analysis: WRegionCollectionAnalysis
;            Running analysis: BlockFrequencyAnalysis
;            Running analysis: BranchProbabilityAnalysis
;            Running analysis: PostDominatorTreeAnalysis
;            Running analysis: LoopAccessAnalysis
;CHECK:      Running pass: MathLibraryFunctionsReplacementPass
;            Running pass: AlwaysInlinerPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPODirectiveCleanupPass
;            Running pass: SROAPass
;            Running pass: SimplifyCFGPass
;CHECK:      Running pass: VPORestoreOperandsPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPOParoptSharedPrivatizationPass
;CHECK:      Running pass: VPOParoptOptimizeDataSharingPass
;            Invalidating analysis: DominatorTreeAnalysis
;            Invalidating analysis: LoopAnalysis
;            Invalidating analysis: ScalarEvolutionAnalysis
;            Invalidating analysis: BasicAA
;            Invalidating analysis: AAManager
;            Invalidating analysis: DemandedBitsAnalysis
;            Invalidating analysis: WRegionCollectionAnalysis
;            Invalidating analysis: WRegionInfoAnalysis
;            Invalidating analysis: PostDominatorTreeAnalysis
;            Invalidating analysis: BranchProbabilityAnalysis
;            Invalidating analysis: BlockFrequencyAnalysis
;            Invalidating analysis: LoopAccessAnalysis
;            Running pass: LoopSimplifyPass
;            Running analysis: LoopAnalysis
;            Running analysis: DominatorTreeAnalysis
;CHECK:      Running pass: VPOParoptPass
;            Running analysis: WRegionInfoAnalysis
;            Running analysis: WRegionCollectionAnalysis
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: AAManager
;            Running analysis: BasicAA
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPODirectiveCleanupPass
;CHECK:      Running pass: VPOCFGSimplifyPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPOParoptGuardMemoryMotionPass
;CHECK:      Running pass: VPOCFGRestructuringPass
;CHECK:      Running pass: VPORenameOperandsPass
;            Running pass: AlwaysInlinerPass
;            Running pass: GlobalDCEPass
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module>
;            Running pass: TightLoopEmitterPass
;            Running analysis: OptReportOptionsAnalysis
;            Running analysis: LoopAnalysis
;            Running analysis: DominatorTreeAnalysis
;            Running pass: LoopDistributePass
;            Running analysis: ScalarEvolutionAnalysis
;            Running analysis: TargetLibraryAnalysis
;            Running analysis: AssumptionAnalysis
;            Running analysis: TargetIRAnalysis
;            Running analysis: OptimizationRemarkEmitterAnalysis
;            Running analysis: LoopAccessAnalysis
;            Running analysis: AAManager
;            Running analysis: BasicAA
;            Running analysis: XmainOptLevelAnalysis
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function>
;            Running analysis: ScopedNoAliasAA
;            Running analysis: TypeBasedAA
;            Running analysis: StdContainerAA
;            Running pass: InjectTLIMappings
;            Running pass: LoopLoadEliminationPass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;CHECK:      Running pass: InstCombinePass
;            Running pass: SimplifyCFGPass
;CHECK:      Running pass: SLPVectorizerPass
;            Running analysis: DemandedBitsAnalysis
;            Running pass: LoadCoalescingPass
;            Running pass: SROAPass
;            Running pass: VectorCombinePass
;            Running pass: EarlyCSEPass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;CHECK:      Running pass: InstCombinePass
;            Running pass: LoopUnrollPass
;            Running pass: SROAPass
;            Running pass: NontemporalStorePass
;            Running pass: WarnMissedTransformationsPass
;CHECK:      Running pass: VPOCFGRestructuringPass ;INTEL
;CHECK:      Running pass: InstCombinePass
;            Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function>
;            Running pass: LoopSimplifyPass
;            Running pass: LCSSAPass
;            Running analysis: MemorySSAAnalysis
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Running pass: LICMPass
;            Running pass: AlignmentFromAssumptionsPass
;            Running pass: LoopSinkPass
;            Running analysis: BlockFrequencyAnalysis
;            Running analysis: BranchProbabilityAnalysis
;            Running analysis: PostDominatorTreeAnalysis
;            Running pass: InstSimplifyPass
;            Running pass: DivRemPairsPass
;            Running pass: TailCallElimPass
;            Running pass: SimplifyCFGPass
;            Invalidating analysis: DominatorTreeAnalysis
;            Invalidating analysis: LoopAnalysis
;            Invalidating analysis: ScalarEvolutionAnalysis
;            Invalidating analysis: BasicAA
;            Invalidating analysis: AAManager
;            Invalidating analysis: LoopAccessAnalysis
;            Invalidating analysis: DemandedBitsAnalysis
;            Invalidating analysis: MemorySSAAnalysis
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function>
;            Invalidating analysis: PostDominatorTreeAnalysis
;            Invalidating analysis: BranchProbabilityAnalysis
;            Invalidating analysis: BlockFrequencyAnalysis
;            Running pass: GlobalDCEPass
;            Running pass: ConstantMergePass
;            Running pass: CGProfilePass
;            Running analysis: BlockFrequencyAnalysis
;            Running analysis: BranchProbabilityAnalysis
;            Running analysis: LoopAnalysis
;            Running analysis: DominatorTreeAnalysis
;            Running analysis: PostDominatorTreeAnalysis
;            Running pass: RelLookupTableConverterPass
;            Running pass: InlineReportEmitterPass
;            Running pass: AnnotationRemarksPass

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
