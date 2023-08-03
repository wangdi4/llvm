; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that both stores to %A are eliminated by propagating to the load after
; unrolling. We verify behavior with unrolling so both (%A)[0][0] and (%A)[0][1]
; have same symbase and we can test the aliasing logic of DSE.

; Incoming HIR to unroll-
; + DO i1 = 0, 499, 1   <DO_LOOP>
; |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll = 2>
; |   |   (%A)[0][i2] = %t;
; |   |   %ld = (%A)[0][i2];
; |   + END LOOP
; + END LOOP

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK: |   (%A)[0][0] = %t;
; CHECK: |   %ld = (%A)[0][0];
; CHECK: |   (%A)[0][1] = %t;
; CHECK: |   %ld = (%A)[0][1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK:      + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK-NEXT: |   %ld = %t;
; CHECK-NEXT: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  br label %outer.loop

outer.loop:
 %outer.iv = phi i64 [ 0, %entry], [ %outer.iv.inc, %outer.latch]
 br label %loop

loop:
  %iv = phi i64 [ 0, %outer.loop], [ %iv.inc, %loop]
  %iv.inc = add i64 %iv, 1
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  store i32 %t, ptr %gep, align 4
  %ld = load i32, ptr %gep, align 4
  %cmp = icmp eq i64 %iv.inc, 2
  br i1 %cmp, label %outer.latch, label %loop, !llvm.loop !0

outer.latch:
  %outer.ld = phi i32 [ %ld, %loop ]
  %outer.iv.inc = add i64 %outer.iv, 1
  %outer.cmp = icmp eq i64 %outer.iv.inc, 500
  br i1 %outer.cmp, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi i32 [ %outer.ld, %outer.latch ]
  ret i32 %ld.lcssa
}

!0 = !{!0, !1}
!1 = !{!"llvm.loop.unroll.count", i32 2}

