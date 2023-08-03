; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we are able to optimize away dead store and load based on
; non-linear base %A by proving that %A is only defined once in the loop and it
; dominates all the uses.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %A = @bar();
; CHECK: |   (%A)[0][i1] = 10;
; CHECK: |   %ld = (%A)[0][i1];
; CHECK: |   (%A)[0][i1] = 20;
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: modified

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %A = @bar();
; CHECK: |   %ld = 10;
; CHECK: |   (%A)[0][i1] = 20;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo() {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %A = call ptr @bar();
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  store i32 10, ptr %gep, align 4
  %ld = load i32, ptr %gep, align 4
  store i32 20, ptr %gep, align 4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 4
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %ld, %loop ]
  ret i32 %ld.lcssa
}

declare ptr @bar() #0

attributes #0 = { nounwind memory(none) }

