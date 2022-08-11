; INTEL_CUSTOMIZATION
; RUN: opt -disable-verify -disable-output -verify-cfg-preserved=0 \
; RUN:     -debug-pass-manager  -passes='default<O2>' \
; RUN:     -paropt=31 -S %s 2>&1 | FileCheck %s

;            Running pass: XmainOptLevelAnalysisInit on [module] ;INTEL
;            Running analysis: XmainOptLevelAnalysis on [module] ;INTEL
;            Running pass: Annotation2MetadataPass on [module]
;            Running pass: ForceFunctionAttrsPass on [module]
;            Running pass: InferFunctionAttrsPass on [module]
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module> on [module]
;            Running analysis: TargetLibraryAnalysis on bar
;CHECK:      Running pass: RequireAnalysisPass<llvm::VPOParoptConfigAnalysis, llvm::Module> on [module] ;INTEL
;CHECK-NEXT: Running analysis: VPOParoptConfigAnalysis on [module]                                      ;INTEL
;            Running pass: InlineReportSetupPass on [module]  ;INTEL
;            Running pass: InlineListsPass on [module]        ;INTEL
;            Running pass: LowerSubscriptIntrinsicPass on foo ;INTEL
;CHECK:      Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running pass: VPOParoptApplyConfigPass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-DAG:  Running analysis: TargetLibraryAnalysis on foo
;CHECK-DAG:  Running analysis: AssumptionAnalysis on foo
;CHECK-DAG:  Running analysis: TargetIRAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis on foo ;INTEL
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*Function.*}}> on foo
;CHECK-NEXT: Running analysis: ScopedNoAliasAA on foo
;CHECK-NEXT: Running analysis: TypeBasedAA on foo
;CHECK-NEXT: Running analysis: StdContainerAA on foo
;CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis on foo
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running pass: VPOParoptLoopTransformPass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running pass: VPOParoptLoopCollapsePass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running pass: LoopSimplifyPass on foo
;CHECK-NEXT: Running pass: VPOParoptPreparePass on foo
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on foo ;INTEL
;            Running pass: LowerExpectIntrinsicPass on foo
;            Running pass: SimplifyCFGPass on foo
;            Running pass: SROA on foo
;            Running pass: EarlyCSEPass on foo
;            Running pass: CoroEarlyPass on foo
;            Invalidating analysis: DominatorTreeAnalysis on foo
;            Invalidating analysis: LoopAnalysis on foo
;            Invalidating analysis: ScalarEvolutionAnalysis on foo
;            Invalidating analysis: BasicAA on foo
;            Invalidating analysis: AAManager on foo
;            Invalidating analysis: WRegionCollectionAnalysis on foo
;            Invalidating analysis: WRegionInfoAnalysis on foo
;            Running pass: IPSCCPPass on [module]
;            Running analysis: DominatorTreeAnalysis on foo
;            Running pass: CalledValuePropagationPass on [module]
;            Running pass: GlobalOptPass on [module]
;            Running pass: PromotePass on foo
;            Running pass: DeadArgumentEliminationPass on [module]
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;            Running analysis: LoopAnalysis on foo
;CHECK:      Running pass: InstCombinePass on foo
;            Running analysis: AAManager on foo
;            Running analysis: BasicAA on foo
;            Running pass: SimplifyCFGPass on foo
;            Invalidating analysis: DominatorTreeAnalysis on foo
;            Invalidating analysis: LoopAnalysis on foo
;            Invalidating analysis: BasicAA on foo
;            Invalidating analysis: AAManager on foo
;CHECK:      Running pass: ModuleInlinerWrapperPass on [module]
;CHECK-NEXT: Running analysis: InlineAdvisorAnalysis on [module]
;CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::GlobalsAA{{.*}}> on [module]
;CHECK-NEXT: Running analysis: GlobalsAA on [module]
;CHECK-NEXT: Running analysis: CallGraphAnalysis on [module]
;CHECK-NEXT: Running pass: InvalidateAnalysisPass<llvm::AAManager> on foo
;CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis{{.*}}> on [module]
;CHECK-NEXT: Running analysis: ProfileSummaryAnalysis on [module]
;CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}> on [module]
;CHECK-NEXT: Running analysis: LazyCallGraphAnalysis on [module]
;CHECK-NEXT: Running analysis: FunctionAnalysisManagerCGSCCProxy on (foo)
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*Module.*}}, {{.*SCC.*}}> on (foo)
;CHECK-NEXT: Running pass: InlinerPass on (foo)
;CHECK-NEXT: Running pass: InlinerPass on (foo)
;CHECK-NEXT: Invalidating analysis: InlineAdvisorAnalysis on [module]
;CHECK-NEXT: Running pass: SROAPass on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running pass: SimplifyCFGPass on foo
;CHECK-NEXT: Running pass: VPORestoreOperandsPass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptSharedPrivatizationPass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: BasicAA on foo
;CHECK-NEXT: Invalidating analysis: AAManager on foo
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis on foo
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: LoopSimplifyPass on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: CallGraphAnalysis on {{.*}}
;CHECK-NEXT: Invalidating analysis: LazyCallGraphAnalysis on {{.*}}
;CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*SCC.*}}, {{.*Module.*}}> on {{.*}}
;CHECK-NEXT: Running pass: VPOParoptPass on [module]
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on [module]
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running pass: VPOCFGSimplifyPass on foo
;CHECK-NEXT: Running pass: AlwaysInlinerPass on [module] ;INTEL
;CHECK-NEXT: Running pass: OpenMPOptPass on [module]
;CHECK-NEXT: Running pass: GlobalDCEPass on [module]     ;INTEL
;CHECK-NEXT: Invalidating analysis: InnerAnalysisManagerProxy<{{.*Function.*}}, {{.*Module.*}}> on {{.*}}
;            Running pass: ModuleInlinerWrapperPass on [module]
;            Running analysis: InlineAdvisorAnalysis on [module]
;            Running analysis: InnerAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Module> on [module]
;            Running pass: RequireAnalysisPass<llvm::GlobalsAA, llvm::Module> on [module]
;            Running pass: InvalidateAnalysisPass<llvm::AAManager> on foo
;            Running pass: RequireAnalysisPass<llvm::ProfileSummaryAnalysis, llvm::Module> on [module]
;            Running analysis: InnerAnalysisManagerProxy<llvm::CGSCCAnalysisManager, llvm::Module> on [module]
;            Running analysis: LazyCallGraphAnalysis on [module]
;            Running analysis: TargetLibraryAnalysis on foo ;INTEL
;            Running analysis: FunctionAnalysisManagerCGSCCProxy on (foo)
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, LazyCallGraph::SCC, llvm::LazyCallGraph &> on (foo)
;            Running pass: DevirtSCCRepeatedPass on (foo)
;            Running pass: InlinerPass on (foo)
;            Running analysis: OptimizationRemarkEmitterAnalysis on foo ;INTEL
;            Running pass: InlinerPass on (foo)
;            Running pass: ArgumentPromotionPass on (foo)
;            Running analysis: TargetIRAnalysis on foo
;            Running pass: SROAPass on foo
;            Running analysis: DominatorTreeAnalysis on foo
;            Running analysis: AssumptionAnalysis on foo
;            Running pass: PostOrderFunctionAttrsPass on (foo)
;            Running analysis: AAManager on foo
;            Running analysis: BasicAA on foo
;            Running analysis: XmainOptLevelAnalysis on foo ;INTEL
;            Running analysis: OuterAnalysisManagerProxy<llvm::ModuleAnalysisManager, llvm::Function> on foo
;            Running analysis: ScopedNoAliasAA on foo
;            Running analysis: TypeBasedAA on foo
;            Running analysis: StdContainerAA on foo
;            Running pass: OpenMPOptCGSCCPass on (foo)
;            Running pass: TbaaMDPropagationPass on foo
;            Running pass: RequireAnalysisPass<llvm::OptReportOptionsAnalysis, llvm::Function> on foo
;            Running analysis: OptReportOptionsAnalysis on foo ;INTEL
;            Running pass: SROAPass on foo
;            Running pass: EarlyCSEPass on foo
;            Running analysis: MemorySSAAnalysis on foo
;            Running pass: SpeculativeExecutionPass on foo
;            Running pass: JumpThreadingPass on foo
;            Running analysis: LazyValueAnalysis on foo
;            Running analysis: PostDominatorTreeAnalysis on foo ;INTEL
;            Running pass: CorrelatedValuePropagationPass on foo
;            Invalidating analysis: LazyValueAnalysis on foo
;            Running pass: SimplifyCFGPass on foo
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;            Running analysis: LoopAnalysis on foo
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: LibCallsShrinkWrapPass on foo
;            Running pass: TailCallElimPass on foo
;            Running pass: SimplifyCFGPass on foo
;            Running pass: ReassociatePass on foo
;            Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function> on foo
;            Running pass: LoopSimplifyPass on foo
;            Running pass: LCSSAPass on foo
;            Running analysis: ScalarEvolutionAnalysis on foo
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function> on foo
;            Running pass: LoopInstSimplifyPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopSimplifyCFGPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LICMPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopRotatePass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LICMPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: SimpleLoopUnswitchPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: SimplifyCFGPass on foo
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: LoopSimplifyPass on foo
;            Running pass: LCSSAPass on foo
;            Running pass: LoopIdiomRecognizePass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: IndVarSimplifyPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopDeletionPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopFullUnrollPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running analysis: OuterAnalysisManagerProxy<llvm::FunctionAnalysisManager, llvm::Loop, llvm::LoopStandardAnalysisResults &> on Loop at depth 1 containing: %loop<header><latch><exiting> ;INTEL
;            Running pass: SROAPass on foo
;            Running pass: MergedLoadStoreMotionPass on foo
;            Running pass: GVNPass on foo
;            Running analysis: MemoryDependenceAnalysis on foo
;            Running analysis: PhiValuesAnalysis on foo
;            Running pass: SCCPPass on foo
;            Running pass: BDCEPass on foo
;            Running analysis: DemandedBitsAnalysis on foo
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: JumpThreadingPass on foo
;            Running analysis: LazyValueAnalysis on foo
;            Running pass: CorrelatedValuePropagationPass on foo
;            Invalidating analysis: LazyValueAnalysis on foo
;            Running pass: ADCEPass on foo
;            Running pass: MemCpyOptPass on foo
;            Running pass: DSEPass on foo
;            Running pass: LoopSimplifyPass on foo
;            Running pass: LCSSAPass on foo
;            Running pass: LICMPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: CoroElidePass on foo
;            Running pass: SimplifyCFGPass on foo
;CHECK:      Running pass: VPOCFGRestructuringPass on foo
;CHECK:      Running pass: InstCombinePass on foo
;            Clearing all analysis results for: <possibly invalidated loop>
;            Invalidating analysis: DominatorTreeAnalysis on foo
;            Invalidating analysis: BasicAA on foo
;            Invalidating analysis: AAManager on foo
;            Invalidating analysis: MemorySSAAnalysis on foo
;            Invalidating analysis: PostDominatorTreeAnalysis on foo
;            Invalidating analysis: LoopAnalysis on foo
;            Invalidating analysis: ScalarEvolutionAnalysis on foo
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function> on foo
;            Invalidating analysis: PhiValuesAnalysis on foo
;            Invalidating analysis: MemoryDependenceAnalysis on foo
;            Invalidating analysis: DemandedBitsAnalysis on foo
;            Running pass: CoroSplitPass on (foo)
;            Invalidating analysis: InlineAdvisorAnalysis on [module]
;            Running pass: IPSCCPPass on [module]
;            Running analysis: DominatorTreeAnalysis on foo
;            Running pass: GlobalOptPass on [module]
;            Running analysis: TargetLibraryAnalysis on bar ;INTEL
;            Running pass: GlobalDCEPass on [module]
;            Running pass: StdContainerOptPass on foo ;INTEL
;            Running analysis: AAManager on foo
;            Running analysis: BasicAA on foo
;            Running pass: CleanupFakeLoadsPass on foo ;INTEL
;            Running pass: EliminateAvailableExternallyPass on [module]
;            Running pass: ReversePostOrderFunctionAttrsPass on [module]
;            Running analysis: CallGraphAnalysis on [module]
;            Running pass: RequireAnalysisPass<llvm::AndersensAA, llvm::Module> on [module] ;INTEL
;            Running analysis: AndersensAA on [module] ;INTEL
;            Running pass: RecomputeGlobalsAAPass on [module]
;            Running pass: Float2IntPass on foo
;            Running pass: LowerConstantIntrinsicsPass on foo
;            Running pass: LoopSimplifyPass on foo
;            Running analysis: LoopAnalysis on foo
;            Running pass: LCSSAPass on foo
;            Running analysis: ScalarEvolutionAnalysis on foo
;            Running analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function> on foo
;            Running pass: LoopRotatePass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopDeletionPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: LoopDistributePass on foo
;            Running pass: InjectTLIMappings on foo
;            Running pass: LoopLoadEliminationPass on foo
;            Running analysis: LoopAccessAnalysis on Loop at depth 1 containing: %loop<header><latch><exiting>
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: SimplifyCFGPass on foo
;            Running pass: SLPVectorizerPass on foo
;            Running analysis: DemandedBitsAnalysis on foo
;            Running pass: VectorCombinePass on foo
;            Running pass: EarlyCSEPass on foo ;INTEL
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: LoopUnrollPass on foo
;            Running pass: WarnMissedTransformationsPass on foo
;CHECK:      Running pass: VPOCFGRestructuringPass on foo ;INTEL
;CHECK:      Running pass: InstCombinePass on foo
;            Running pass: RequireAnalysisPass<llvm::OptimizationRemarkEmitterAnalysis, llvm::Function> on foo
;            Running pass: LoopSimplifyPass on foo
;            Running pass: LCSSAPass on foo
;            Running analysis: MemorySSAAnalysis on foo
;            Running pass: LICMPass on Loop at depth 1 containing: %loop<header><latch><exiting>
;            Running pass: AlignmentFromAssumptionsPass on foo
;            Running pass: LoopSinkPass on foo
;            Running analysis: BlockFrequencyAnalysis on foo
;            Running analysis: BranchProbabilityAnalysis on foo
;            Running analysis: PostDominatorTreeAnalysis on foo
;            Running pass: InstSimplifyPass on foo
;            Running pass: DivRemPairsPass on foo
;            Running pass: SimplifyCFGPass on foo
;            Running pass: CoroCleanupPass on foo
;            Clearing all analysis results for: <possibly invalidated loop>
;            Invalidating analysis: DominatorTreeAnalysis on foo
;            Invalidating analysis: LoopAnalysis on foo
;            Invalidating analysis: BasicAA on foo
;            Invalidating analysis: AAManager on foo
;            Invalidating analysis: ScalarEvolutionAnalysis on foo
;            Invalidating analysis: InnerAnalysisManagerProxy<llvm::LoopAnalysisManager, llvm::Function> on foo
;            Invalidating analysis: PostDominatorTreeAnalysis on foo
;            Invalidating analysis: BranchProbabilityAnalysis on foo
;            Invalidating analysis: BlockFrequencyAnalysis on foo
;            Invalidating analysis: DemandedBitsAnalysis on foo
;            Invalidating analysis: MemorySSAAnalysis on foo
;            Running pass: CGProfilePass on [module]
;            Running analysis: BlockFrequencyAnalysis on foo
;            Running analysis: BranchProbabilityAnalysis on foo
;            Running analysis: LoopAnalysis on foo
;            Running analysis: DominatorTreeAnalysis on foo
;            Running analysis: PostDominatorTreeAnalysis on foo
;            Running pass: GlobalDCEPass on [module]
;            Running pass: ConstantMergePass on [module]
;            Running pass: RelLookupTableConverterPass on [module]
;            Running analysis: TargetIRAnalysis on bar
;            Running pass: InlineReportEmitterPass on [module] ;INTEL
;            Running pass: AnnotationRemarksPass on foo

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
