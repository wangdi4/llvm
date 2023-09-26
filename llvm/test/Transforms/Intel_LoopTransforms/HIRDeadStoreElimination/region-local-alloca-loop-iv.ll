; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to (%A)[0][i1] is eliminated by propagating to the load in
; the presence of store (%A)[0][i1 + 1] which has the same symbase. The other
; store is also eliminated as %A alloca is local to the region.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][i1] = %t;
; CHECK: |   (%A)[0][i1 + 1] = %t;
; CHECK: |   %ld = (%A)[0][i1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %ld = %t;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %iv.inc = add i64 %iv, 1
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  %gep1 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv.inc
  store i32 %t, ptr %gep, align 4
  store i32 %t, ptr %gep1, align 4
  %ld = load i32, ptr %gep, align 4
  %cmp = icmp eq i64 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %ld, %loop ]
  ret i32 %ld.lcssa
}


