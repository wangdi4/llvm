; INTEL_COLLAB
; RUN: opt -disable-verify -disable-output -verify-cfg-preserved=0 \
; RUN:     -debug-pass-manager  -passes='default<O0>' \
; RUN:     -paropt=31 -S %s 2>&1 | FileCheck %s

; INTEL_CUSTOMIZATION
;CHECK:      Running pass: XmainOptLevelAnalysisInit on [module]
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis on [module]
;CHECK-NEXT: Running pass: RequireAnalysisPass<llvm::VPOParoptConfigAnalysis, llvm::Module> on [module]
;CHECK-NEXT: Running analysis: VPOParoptConfigAnalysis on [module]
;CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}> on [module]
;CHECK-NEXT: Running pass: LowerSubscriptIntrinsicPass on foo
; end INTEL_CUSTOMIZATION
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
;CHECK:      Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis on foo
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*}}> on foo
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: ScopedNoAliasAA on foo
;CHECK-NEXT: Running analysis: TypeBasedAA on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: StdContainerAA on foo
; end INTEL_CUSTOMIZATION
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
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on foo
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: InlineListsPass on [module]
;CHECK-NEXT: Running pass: AlwaysInlinerPass on [module]
;CHECK-NEXT: Running analysis: ProfileSummaryAnalysis on [module]
;CHECK-NEXT: Running pass: VPORestoreOperandsPass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: BasicAA on foo
;CHECK-NEXT: Invalidating analysis: AAManager on foo
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running pass: LoopSimplifyPass on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptPass on [module]
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on [module]
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running pass: VPODirectiveCleanupPass on foo
;CHECK-NEXT: Running pass: VPOCFGSimplifyPass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running pass: VPOParoptGuardMemoryMotionPass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running pass: VPORenameOperandsPass on foo
;CHECK-NEXT: Running pass: AlwaysInlinerPass on [module]
;            Running pass: CoroEarlyPass on foo
;            Running analysis: InnerAnalysisManagerProxy<{{.*}}> on [module]
;            Running analysis: LazyCallGraphAnalysis on [module]
;            Running analysis: FunctionAnalysisManagerCGSCCProxy on (foo)
;            Running analysis: OuterAnalysisManagerProxy<{{.*}}> on (foo)
;            Running pass: CoroSplitPass on (foo)
;            Running pass: CoroCleanupPass on foo
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
; end INTEL_COLLAB
