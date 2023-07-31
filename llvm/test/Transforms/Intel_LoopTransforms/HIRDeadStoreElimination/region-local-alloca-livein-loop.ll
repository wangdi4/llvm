; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to (%A)[0][0] is not eliminated as it has a loop-carried
; use in the load.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][i1] = 2;
; CHECK: |
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   %ld = (%A)[0][0];
; CHECK: |   |   (%A)[0][0] = %t;
; CHECK: |   |   %ld1 = (%A)[0][0];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][i1] = 2;
; CHECK: |
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   %ld = (%A)[0][0];
; CHECK: |   |   (%A)[0][0] = %t;
; CHECK: |   |   %ld1 = (%A)[0][0];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %bc = bitcast ptr %A to ptr
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 0
  br label %outer.loop

outer.loop:
  %iv.outer = phi i64 [ 0, %entry], [ %iv.outer.inc, %latch ]
  %gep1 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv.outer
  store i32 2, ptr %gep1, align 4
  br label %loop

loop:
  %iv = phi i32 [ 0, %outer.loop], [ %iv.inc, %loop]
  %ld = load i32, ptr %gep, align 4
  store i32 %t, ptr %gep, align 4
  %ld1 = load i32, ptr %gep, align 4
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %latch, label %loop

latch:
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp1 = icmp eq i64 %iv.outer.inc, 5
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi i32 [ %ld1, %latch ]
  ret i32 %ld.lcssa
}

