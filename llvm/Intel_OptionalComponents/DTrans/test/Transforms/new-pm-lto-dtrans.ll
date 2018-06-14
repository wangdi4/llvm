; Test DTrans integration in the new pass manager.
;
; Ideally, we'd like to have this test integrated with the main test for the
; new pass manager pipeline (test/Other/new-pm-lto-default.ll) but in order
; to exclude the use of the dtrans option for builds that exclude dtrans, the
; test must be separate.

; RUN: opt -disable-verify -debug-pass-manager \
; RUN:     -passes='lto<O2>' -S  %s -enable-npm-dtrans 2>&1 \
; RUN:     | FileCheck %s

; Basic orientation checks.
; CHECK: Starting llvm::Module pass manager run.
; CHECK-NEXT: Running pass: PassManager<{{.*}}Module
; CHECK-NEXT: Starting llvm::Module pass manager run.

; CHECK: Running analysis: CallGraphAnalysis
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}InstSimplifierPass{{.*}}>
; CHECK: Running analysis: DominatorTreeAnalysis
; CHECK: Running analysis: AssumptionAnalysis
; CHECK-NEXT: Running pass: ModuleToFunctionPassAdaptor<{{.*}}SimplifyCFGPass{{.*}}>
; CHECK-NEXT: Running pass: dtrans::DeleteFieldPass
; CHECK-NEXT: Running analysis: DTransAnalysis
; CHECK-NEXT: Running analysis: BlockFrequencyAnalysis on foo
; CHECK-NOT: Running analysis: DTransAnalysis
; CHECK: Running pass: dtrans::AOSToSOAPass
; CHECK-NEXT: Running pass: dtrans::ReorderFieldsPass
; CHECK-NEXT: Running pass: OptimizeDynamicCastsPass
; CHECK-NEXT: Running analysis: WholeProgramAnalysis

; Make sure we get the IR back out without changes when we print the module.
; CHECK-LABEL: define void @foo(i32 %n) local_unnamed_addr {
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
; CHECK-NEXT: Finished llvm::Module pass manager run.

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
