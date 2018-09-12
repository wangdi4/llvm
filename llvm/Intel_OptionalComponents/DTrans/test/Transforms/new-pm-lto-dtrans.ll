; Test DTrans integration in the new pass manager.
;
; Ideally, we'd like to have this test integrated with the main test for the
; new pass manager pipeline (test/Other/new-pm-lto-default.ll) but in order
; to exclude the use of the dtrans option for builds that exclude dtrans, the
; test must be separate.

; RUN: opt -disable-verify -debug-pass-manager -whole-program-assume \
; RUN:     -passes='lto<O2>' -S  %s -enable-npm-dtrans 2>&1 \
; RUN:     | FileCheck %s

; Basic orientation checks.
; CHECK: Starting llvm::Module pass manager run.
; CHECK-NEXT: Running pass: PassManager<{{.*}}Module
; CHECK-NEXT: Starting llvm::Module pass manager run.

; CHECK: Running analysis: TargetLibraryAnalysis
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}Function{{.*}}>
; CHECK: Running analysis: DominatorTreeAnalysis
; CHECK: Running analysis: AssumptionAnalysis
; CHECK: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}InstSimplifyPass{{.*}}>
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}SimplifyCFGPass{{.*}}>
; CHECK-NEXT: Running pass: dtrans::ResolveTypes
; CHECK: Running analysis: DTransAnalysis
; The ordering of the analysis passes seems not to be deterministic so we
; don't check them all here. The check below guarantees that DeleteFieldPass
; is the next non-analysis pass to run.
; CHECK: Running pass:
; CHECK-SAME: dtrans::DeleteFieldPass
; Now we switch to CHECK-NEXT to make sure the analysis passes aren't re-run.
; CHECK-NEXT: Running pass: dtrans::ReorderFieldsPass
; CHECK-NEXT: Running pass: dtrans::AOSToSOAPass
; CHECK-NEXT: Running pass: dtrans::EliminateROFieldAccessPass
; CHECK-NEXT: Running pass: dtrans::DynClonePass
; CHECK-NEXT: Running pass: dtrans::SOAToAOSPass
; CHECK-NEXT: Running pass: OptimizeDynamicCastsPass

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
; CHECK-LABEL: define i32 @main() local_unnamed_addr {
; CHECK-NEXT:    call fastcc void @foo(i32 1)
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

define i32 @main() {
  call void @foo(i32 1)
  ret i32 0
}

attributes #0 = { noinline uwtable }
