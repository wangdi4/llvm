; Test DTrans integration in the new pass manager.
;
; Ideally, we'd like to have this test integrated with the main test for the
; new pass manager pipeline (test/Other/new-pm-lto-default.ll) but in order
; to exclude the use of the dtrans option for builds that exclude dtrans, the
; test must be separate.

; RUN: opt -disable-verify -debug-pass-manager -whole-program-assume    \
; RUN:     -enable-dtrans-soatoaos -enable-dtrans-deletefield           \
; RUN:     -enable-resolve-types                                        \
; RUN:     -passes='lto<O2>,internalize'  -internalize-public-api-list main \
; RUN:     -S  %s -enable-npm-dtrans                  \
; RUN:     2>&1 \
; RUN:     | FileCheck %s

; Basic orientation checks.
; CHECK: Running analysis: PassInstrumentationAnalysis
; CHECK-NEXT: Starting llvm::Module pass manager run.
; CHECK-NEXT: Running pass: PassManager<{{.*}}Module
; CHECK-NEXT: Starting llvm::Module pass manager run.
; CHECK-NEXT: Running pass: InlineReportSetupPass
; CHECK-NEXT: Running pass: XmainOptLevelAnalysisInit
; CHECK-NEXT: Running analysis: XmainOptLevelAnalysis
; CHECK-NEXT: GlobalDCEPass
; CHECK: Running analysis: WholeProgramAnalysis
; CHECK: Running analysis: TargetLibraryAnalysis
; CHECK: Running pass: InternalizePass
; CHECK: Running pass: IPSCCPPass
; CHECK-NEXT: Running analysis: InnerAnalysisManagerProxy<{{.*}}Module{{.*}}>
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis on foo
; CHECK-NEXT: Running analysis: PassInstrumentationAnalysis on foo
; CHECK-NEXT: Running analysis: AssumptionAnalysis on foo
; CHECK-NEXT: Running analysis: DominatorTreeAnalysis on main
; CHECK-NEXT: Running analysis: PassInstrumentationAnalysis on main
; CHECK-NEXT: Running analysis: AssumptionAnalysis on main
; CHECK-NEXT: Running pass: IPCloningPass
; CHECK-NEXT: Running pass: ForceFunctionAttrsPass
; CHECK-NEXT: Running pass: InferFunctionAttrsPass
; CHECK: Running pass: ModuleToFunctionPassAdaptor<{{.*}}InstSimplifyPass{{.*}}>
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}SimplifyCFGPass{{.*}}>

; Verify that resolve types does not invoke DTransAnalysis
; CHECK-NEXT: Running pass: dtrans::ResolveTypes
; CHECK-NOT: Running analysis: DTransAnalysis
; CHECK: Running pass: dtrans::TransposePass
; CHECK: Running pass: dtrans::SOAToAOSPass
; The ordering of the analysis passes seems not to be deterministic so we
; don't check them all here. The check below guarantees that WeakAlignPass
; is the next non-analysis pass to run.
; CHECK: Running analysis: DTransAnalysis
; Now we switch to CHECK-NEXT to make sure the analysis passes aren't re-run.
; CHECK: Running pass:
; CHECK-SAME: dtrans::WeakAlignPass
; CHECK-NEXT: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running analysis: PassInstrumentationAnalysis
; CHECK-NEXT: Running pass: dtrans::DeleteFieldPass
; CHECK-NEXT: Running pass: dtrans::MemInitTrimDownPass
; CHECK-NEXT: Running pass: dtrans::ReorderFieldsPass
; CHECK-NEXT: Running pass: dtrans::AOSToSOAPass
; CHECK-NEXT: Running pass: dtrans::EliminateROFieldAccessPass
; CHECK-NEXT: Running pass: dtrans::DynClonePass
; CHECK-NEXT: Running pass: dtrans::AnnotatorCleaner
; CHECK-NEXT: Running pass: DopeVectorConstProp
; CHECK-NEXT: Running pass: OptimizeDynamicCastsPass
; CHECK: Running pass: IntelArgumentAlignmentPass

; Make sure we get the IR back out without changes when we print the module.
; CHECK-LABEL: define internal fastcc void @foo(i32 %n) unnamed_addr #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %loop
; CHECK:      loop:
; CHECK-NEXT:   %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
; CHECK-NEXT:   %iv.next = add i32 %iv, 1
; CHECK-NEXT:   tail call void @bar()
; CHECK-NEXT:   %cmp = icmp eq i32 %iv, %n
; CHECK-NEXT:   br i1 %cmp, label %exit, label %loop
; CHECK:      exit:
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
;
; CHECK-LABEL: define i32 @main(i32 %n) local_unnamed_addr {
; CHECK-NEXT:    call fastcc void @foo(i32 %n)
; CHECK-NEXT:    ret i32 0
; CHECK-NEXT:  }
;
; CHECK-LABEL: attributes #0 = { noinline uwtable }
; CHECK-NEXT: Finished llvm::Module pass manager run.

declare void @bar() local_unnamed_addr

; Function Attrs: noinline uwtable
define void @foo(i32 %n) local_unnamed_addr #0 {
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

define i32 @main(i32 %n) {
  call void @foo(i32 %n)
  ret i32 0
}

attributes #0 = { noinline uwtable }
