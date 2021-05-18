; INTEL_COLLAB
; RUN: opt -disable-verify -disable-output -verify-cfg-preserved=0 \
; RUN:     -debug-pass-manager  -passes='default<O0>' \
; RUN:     -paropt=31 -S %s 2>&1 | FileCheck %s

;            Starting llvm::Module pass manager run.
; INTEL_CUSTOMIZATION
;CHECK:      Running pass: XmainOptLevelAnalysisInit on [module]
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis on [module]
;CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}> on [module]
; end INTEL_CUSTOMIZATION
;CHECK:      Starting llvm::Function pass manager run.
;CHECK:      Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
;CHECK-NEXT: Running pass: VPOParoptLoopCollapsePass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-DAG:  Running analysis: TargetLibraryAnalysis on foo
;CHECK-DAG:  Running analysis: AssumptionAnalysis on foo
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
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: BasicAA on foo
;CHECK-NEXT: Invalidating analysis: AAManager on foo
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running pass: LoopSimplifyPass on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running pass: VPOParoptPreparePass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running analysis: TargetIRAnalysis on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on foo
;CHECK-NEXT: Finished llvm::Function pass manager run
;            Running pass: InlineListsPass on [module]
;            Running pass: AlwaysInlinerPass on [module]
;            Running analysis: ProfileSummaryAnalysis on [module]
;            Running pass: VecClonePass on [module]
;            Invalidating analysis: InnerAnalysisManagerProxy<{{.*}}>
;CHECK:      Running analysis: InnerAnalysisManagerProxy<{{.*}}> on [module]
;CHECK-NEXT: Starting llvm::Function pass manager run
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPORestoreOperandsPass on foo
;CHECK-NEXT: Running pass: VPOCFGRestructuringPass on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptOptimizeDataSharingPass on foo
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-DAG:  Running analysis: TargetLibraryAnalysis on foo
;CHECK-DAG:  Running analysis: AssumptionAnalysis on foo
;CHECK:      Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running analysis: XmainOptLevelAnalysis on foo
;CHECK-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*}}> on foo
;CHECK-NEXT: Running analysis: ScopedNoAliasAA on foo
;CHECK-NEXT: Running analysis: TypeBasedAA on foo
;CHECK-NEXT: Running analysis: StdContainerAA on foo
;CHECK-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis on foo
;CHECK-NEXT: Invalidating analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Invalidating analysis: LoopAnalysis on foo
;CHECK-NEXT: Invalidating analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: BasicAA on foo
;CHECK-NEXT: Invalidating analysis: AAManager on foo
;CHECK-NEXT: Invalidating analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Invalidating analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running pass: LoopSimplifyPass on foo
;CHECK-NEXT: Running analysis: LoopAnalysis on foo
;CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
;CHECK-NEXT: Finished llvm::Function pass manager run.
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running pass: VPOParoptPass on [module]
;CHECK-NEXT: Running analysis: VPOParoptConfigAnalysis on [module]
; INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: OptReportOptionsAnalysis on [module]
; end INTEL_CUSTOMIZATION
;CHECK-NEXT: Running analysis: WRegionInfoAnalysis on foo
;CHECK-NEXT: Running analysis: WRegionCollectionAnalysis on foo
;CHECK-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
;CHECK-NEXT: Running analysis: AAManager on foo
;CHECK-NEXT: Running analysis: BasicAA on foo
;CHECK-NEXT: Running analysis: TargetIRAnalysis on foo
;CHECK-NEXT: Running pass: VPODirectiveCleanupPass on foo
;CHECK-NEXT: Running pass: AlwaysInlinerPass on [module]
;            Running pass: PrintModulePass on [module]

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
