; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-general-unroll,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we give up on optimizing away store (%A)[0][0] because non-linear
; base %A is redefined.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP> <nounroll>
; CHECK: |   %A = @bar();
; CHECK: |   (%A)[0][0] = 10;
; CHECK: |   %ld = (%A)[0][0];
; CHECK: |   %A = @bar();
; CHECK: |   (%A)[0][0] = 10;
; CHECK: |   %ld = (%A)[0][0];
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: modified

; CHECK: + DO i1 = 0, 1, 1   <DO_LOOP> <nounroll>
; CHECK: |   %A = @bar();
; CHECK: |   (%A)[0][0] = 10;
; CHECK: |   %ld = (%A)[0][0];
; CHECK: |   %A = @bar();
; CHECK: |   (%A)[0][0] = 10;
; CHECK: |   %ld = (%A)[0][0];
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo() {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %A = call ptr @bar();
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 0
  store i32 10, ptr %gep, align 4
  %ld = load i32, ptr %gep, align 4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 4
  br i1 %cmp, label %exit, label %loop, !llvm.loop !0

exit:
  %ld.lcssa = phi i32 [ %ld, %loop ]
  ret i32 %ld.lcssa
}

declare ptr @bar() #0

attributes #0 = { nounwind memory(none) }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}

