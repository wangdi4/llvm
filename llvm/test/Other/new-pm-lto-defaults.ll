; Basic test for the new LTO pipeline.
; For now the only difference is between -O1 and everything else, so
; -O2, -O3, -Os, -Oz are the same.

; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<O1>' -S %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O1
; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<O2>' -S  %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O23SZ \
; RUN:     --check-prefix=CHECK-O2
; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<O3>' -S  %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O23SZ \
; RUN:     --check-prefix=CHECK-O3
; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<Os>' -S %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O23SZ \
; RUN:     --check-prefix=CHECK-OS
; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<Oz>' -S %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O23SZ
; RUN: opt -disable-verify -verify-cfg-preserved=0 -debug-pass-manager \
; RUN:     -passes='lto<O3>' -S  %s -passes-ep-peephole='no-op-function' 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-O --check-prefix=CHECK-O23SZ \
; RUN:     --check-prefix=CHECK-O3 --check-prefix=CHECK-EP-Peephole

; CHECK-O: Starting llvm::Module pass manager run.
; CHECK-O-NEXT: Running pass: Annotation2Metadata
; CHECK-O-NEXT: Running pass: InlineReportSetupPass ;INTEL
; CHECK-O-NEXT: Running pass: XmainOptLevelAnalysisInit ;INTEL
; CHECK-O-NEXT: Running analysis: XmainOptLevelAnalysis ;INTEL
; INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running pass: RequireAnalysisPass<{{.*}}WholeProgramAnalysis
; CHECK-O-NEXT: Running analysis: WholeProgramAnalysis
; CHECK-O-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}Function
; CHECK-O-NEXT: Running analysis: TargetIRAnalysis
; end INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running pass: GlobalDCEPass
; INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running pass: IntelFoldWPIntrinsicPass
; CHECK-O-NEXT: Running pass: IPCloningPass
; end INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running pass: ForceFunctionAttrsPass
; CHECK-O-NEXT: Running pass: InferFunctionAttrsPass
; INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running analysis: TargetLibraryAnalysis
; The TargetLibraryAnalysis is required by the Intel WholeProgramAnalysis.
; It will run during O1. The following CHECK won't be executed.
; CHECK-O-NEXT-: Running analysis: InnerAnalysisManagerProxy<{{.*}}Module
; CHECK-O-NEXT-: Running analysis: TargetLibraryAnalysis
; The InnerAnalysisManagerProxy and the PassInstrumentationAnalysis is needed
; for the Intel WholeProgramAnalysis. It will run with O1. The following CHECK
; won't be executed. The following two CHECKs won't be executed.
; CHECK-O23SZ-NEXT-: Running analysis: PassInstrumentationAnalysis
; end INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Starting llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Running pass: CallSiteSplittingPass on foo
; CHECK-O23SZ-NEXT: Running analysis: TargetLibraryAnalysis on foo
; INTEL_CUSTOMIZATION
; The TargetIRAnalysis isn't needed for the Intel WholeProgramAnalysis.
; It will run with O1. The following CHECKs won't be executed.
; CHECK-O23SZ-NEXT-: Running analysis: TargetIRAnalysis on foo
; end INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running analysis: DominatorTreeAnalysis on foo
; INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: IntelLoopAttrsPass on foo
; CHECK-O23SZ-NEXT: Running analysis: LoopAnalysis on foo
; CHECK-O23SZ-NEXT: Running analysis: ScalarEvolutionAnalysis on foo
; CHECK-O23SZ-NEXT: Running analysis: AssumptionAnalysis on foo
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Finished llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: PGOIndirectCallPromotion
; CHECK-O23SZ-NEXT: Running analysis: ProfileSummaryAnalysis
; CHECK-O23SZ-NEXT: Running analysis: OptimizationRemarkEmitterAnalysis
; CHECK-O23SZ-NEXT: Running pass: IPSCCPPass
; INTEL_CUSTOMIZATION
; The AssumptionAnalysis pass runs with the IntelLoopAttrs pass.
; CHECK-O23SZ-NEXT-: Running analysis: AssumptionAnalysis on foo
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: CalledValuePropagationPass
; CHECK-O-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}SCC
; CHECK-O-NEXT: Running analysis: LazyCallGraphAnalysis
; CHECK-O1-NEXT: Running analysis: TargetLibraryAnalysis
; INTEL_CUSTOMIZATION
; The PassInstrumentationAnalysis isn't needed for the Intel
; WholeProgramAnalysis. It should run at O1. The following CHECKs
; won't be executed.
; CHECK-O1-NEXT-: Running analysis: PassInstrumentationAnalysis
; end INTEL_CUSTOMIZATION
; CHECK-O-NEXT: Running analysis: FunctionAnalysisManagerCGSCCProxy
; CHECK-O-NEXT: Running analysis: OuterAnalysisManagerProxy<{{.*}}LazyCallGraph{{.*}}>
; CHECK-O-NEXT: Running pass: PostOrderFunctionAttrsPass
; CHECK-O-NEXT: Running analysis: AAManager
; CHECK-O-NEXT: Running analysis: BasicAA
; CHECK-O-NEXT: Running analysis: XmainOptLevelAnalysis ;INTEL
; CHECK-O-NEXT: Running analysis: OuterAnalysisManagerProxy ;INTEL
; CHECK-O1-NEXT: Running analysis: AssumptionAnalysis on foo
; CHECK-O1-NEXT: Running analysis: DominatorTreeAnalysis
; CHECK-O-NEXT: Running analysis: ScopedNoAliasAA ;INTEL
; CHECK-O-NEXT: Running analysis: TypeBasedAA
; CHECK-O-NEXT: Running analysis: StdContainerAA
; Moved up to after XmainOptLevelAnalysis ;INTEL
; COM: CHECK-O-NEXT: Running analysis: OuterAnalysisManagerProxy ;INTEL
; CHECK-O-NEXT: Running pass: ReversePostOrderFunctionAttrsPass
; CHECK-O-NEXT: Running analysis: CallGraphAnalysis
; CHECK-O-NEXT: Running pass: DopeVectorConstPropPass ;INTEL
; CHECK-O-NEXT: Running pass: OptimizeDynamicCastsPass ;INTEL
; CHECK-O-NEXT: Running pass: GlobalSplitPass
; CHECK-O-NEXT: Running pass: WholeProgramDevirtPass
; CHECK-O1-NEXT: Running pass: LowerTypeTestsPass
; CHECK-O23SZ-NEXT: Running pass: GlobalOptPass
; CHECK-O23SZ-NEXT: Running pass: PromotePass
; CHECK-O23SZ-NEXT: Running pass: ConstantMergePass
; CHECK-O23SZ-NEXT: Running pass: DeadArgumentEliminationPass
; CHECK-O23SZ-NEXT: Starting llvm::Function pass manager run.
; CHECK-O3-NEXT: Running pass: AggressiveInstCombinePass
; CHECK-O23SZ-NEXT: Running pass: InstCombinePass
; CHECK-EP-Peephole-NEXT: Running pass: NoOpFunctionPass
; CHECK-O23SZ-NEXT: Finished llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Running pass: InlineListsPass ;INTEL
; CHECK-O23SZ-NEXT: Running pass: RequireAnalysisPass<{{.*}}AndersensAA ;INTEL
; CHECK-O23SZ-NEXT: Running analysis: AndersensAA ;INTEL
; CHECK-O23SZ-NEXT: Running pass: IndirectCallConvPass ;INTEL
; CHECK-O23SZ-NEXT: Running pass: AggInlinerPass ;INTEL
; CHECK-O23SZ-NEXT: Running pass: ModuleInlinerWrapperPass
; CHECK-O23SZ-NEXT: Running analysis: InlineAdvisorAnalysis
; CHECK-O23SZ-NEXT: Starting llvm::Module pass manager run.
; CHECK-O23SZ-NEXT: Starting CGSCC pass manager run.
; CHECK-O23SZ-NEXT: Running pass: InlinerPass
; CHECK-O23SZ-NEXT: Running pass: InlinerPass
; CHECK-O23SZ-NEXT: Finished CGSCC pass manager run.
; CHECK-O23SZ-NEXT: Finished llvm::Module pass manager run.
; INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: GlobalOptPass
; CHECK-O23SZ-NEXT: Running pass: IntelIPOPrefetchPass
; CHECK-O23SZ-NEXT: Running pass: PartialInlinerPass
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: IPCloningPass ;INTEL
; CHECK-O23SZ-NEXT: Running pass: GlobalDCEPass
; CHECK-O23SZ-NEXT: Running pass: IPArrayTransposePass ;INTEL
; CHECK-O23SZ-NEXT: Running pass: DeadArrayOpsEliminationPass ;INTEL
; CHECK-O23SZ-NEXT: Starting llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Running pass: InstCombinePass
; CHECK-EP-Peephole-NEXT: Running pass: NoOpFunctionPass
; CHECK-O23SZ-NEXT: Running pass: JumpThreadingPass
; CHECK-O23SZ-NEXT: Running analysis: LazyValueAnalysis
; CHECK-O23SZ-NEXT: Running analysis: PostDominatorTreeAnalysis on foo ;INTEL
; CHECK-O23SZ-NEXT: Running pass: SROA on foo
; CHECK-O23SZ-NEXT: Running pass: TailCallElimPass on foo
; INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: IntelLoopAttrsPass on foo
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Finished llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Running pass: PostOrderFunctionAttrsPass on (foo)
; CHECK-O23SZ-NEXT: Running pass: LoopSimplifyPass on foo
; INTEL_CUSTOMIZATION
; LoopAnalysis will earlier with the IntelLoopAttrsPass
; CHECK-O23SZ-NEXT-: Running analysis: LoopAnalysis on foo
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running pass: LCSSAPass on foo
; INTEL_CUSTOMIZATION
; ScalarEvolution will run with the IntelLoopAttrs pass
; CHECK-O23SZ-NEXT-: Running analysis: ScalarEvolutionAnalysis on foo
; END INTEL_CUSTOMIZATION
; CHECK-O23SZ-NEXT: Running analysis: InnerAnalysisManagerProxy
; Running analysis: PostDominatorTreeAnalysis on foo ;INTEL PostDom has moved, cannot make check work
; CHECK-O23SZ-NEXT: Running pass: LICMPass on Loop
; CHECK-O23SZ-NEXT: Running pass: GVN on foo
; CHECK-O23SZ-NEXT: Running analysis: MemoryDependenceAnalysis on foo
; CHECK-O23SZ-NEXT: Running analysis: PhiValuesAnalysis on foo
; CHECK-O23SZ-NEXT: Running pass: DopeVectorHoistPass on foo ;INTEL
; CHECK-O23SZ-NEXT: Running pass: MemCpyOptPass on foo
; CHECK-O23SZ-NEXT: Running analysis: MemorySSAAnalysis on foo
; CHECK-O23SZ-NEXT: Running pass: DSEPass on foo
; Running analysis: PostDominatorTreeAnalysis on foo   ; INTEL not needed
; CHECK-O23SZ-NEXT: Running pass: MergedLoadStoreMotionPass on foo
; CHECK-O23SZ-NEXT: Starting llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Running pass: LoopSimplifyPass on foo
; CHECK-O23SZ-NEXT: Running pass: LCSSAPass on foo
; CHECK-O23SZ-NEXT: Finished llvm::Function pass manager run.
; CHECK-O23SZ-NEXT: Starting Loop pass manager run.
; CHECK-O23SZ-NEXT: Running pass: IndVarSimplifyPass on Loop
; CHECK-O23SZ-NEXT: Running pass: LoopDeletionPass on Loop
; CHECK-O23SZ-NEXT: Running pass: LoopFullUnrollPass on Loop
; CHECK-O23SZ-NEXT: Running analysis:  OuterAnalysisManagerProxy{{.*}}on Loop at depth 1 containing: %loop<header><latch><exiting> ;INTEL
; CHECK-O23SZ-NEXT: Finished Loop pass manager run.
; CHECK-O23SZ-NEXT: Running pass: LoopDistributePass on foo
; CHECK-O23SZ-NEXT: Running pass: LoopVectorizePass on foo
; CHECK-O23SZ-NEXT: Running analysis: BlockFrequencyAnalysis on foo
; CHECK-O23SZ-NEXT: Running analysis: BranchProbabilityAnalysis on foo
; CHECK-O23SZ-NEXT: Running analysis: DemandedBitsAnalysis on foo
; CHECK-O23SZ-NEXT: Running pass: LoopUnrollPass on foo
; CHECK-O23SZ-NEXT: OptReportOptionsAnalysis on foo ;INTEL
; CHECK-O23SZ-NEXT: WarnMissedTransformationsPass on foo
; CHECK-O23SZ-NEXT: Running pass: InstCombinePass on foo
; CHECK-O23SZ-NEXT: Running pass: SimplifyCFGPass on foo
; CHECK-O23SZ-NEXT: Running pass: SCCPPass on foo
; CHECK-O23SZ-NEXT: Running pass: InstCombinePass on foo
; CHECK-O23SZ-NEXT: Running pass: BDCEPass on foo
; CHECK-O2-NEXT: Running pass: SLPVectorizerPass on foo
; CHECK-O3-NEXT: Running pass: SLPVectorizerPass on foo
; CHECK-OS-NEXT: Running pass: SLPVectorizerPass on foo
; CHECK-O23SZ-NEXT: Running pass: VectorCombinePass on foo
; CHECK-O23SZ-NEXT: Running pass: AlignmentFromAssumptionsPass on foo
; CHECK-O23SZ-NEXT: Running pass: InstCombinePass on foo
; CHECK-EP-Peephole-NEXT: Running pass: NoOpFunctionPass on foo
; CHECK-O23SZ-NEXT: Running pass: JumpThreadingPass on foo
; CHECK-O23SZ-NEXT: Running pass: CrossDSOCFIPass
; CHECK-O23SZ-NEXT: Running pass: LowerTypeTestsPass
; CHECK-O-NEXT: Running pass: LowerTypeTestsPass
; CHECK-O23SZ-NEXT: Running pass: SimplifyCFGPass
; CHECK-O23SZ-NEXT: Running pass: EliminateAvailableExternallyPass
; CHECK-O23SZ-NEXT: Running pass: GlobalDCEPass
; CHECK-O-NEXT: Running pass: AnnotationRemarksPass on foo
; CHECK-O23SZ-NEXT: Running pass: InlineReportEmitterPass ;INTEL
; CHECK-O-NEXT: Running pass: PrintModulePass

; Make sure we get the IR back out without changes when we print the module.
; CHECK-O-LABEL: define void @foo(i32 %n) local_unnamed_addr {
; CHECK-O-NEXT: entry:
; CHECK-O-NEXT:   br label %loop
; CHECK-O:      loop:
; CHECK-O-NEXT:   %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
; CHECK-O-NEXT:   %iv.next = add i32 %iv, 1
; CHECK-O-NEXT:   tail call void @bar()
; CHECK-O-NEXT:   %cmp = icmp eq i32 %iv, %n
; CHECK-O-NEXT:   br i1 %cmp, label %exit, label %loop
; CHECK-O:      exit:
; CHECK-O-NEXT:   ret void
; CHECK-O-NEXT: }
;
; CHECK-O-NEXT: Finished llvm::Module pass manager run.

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
