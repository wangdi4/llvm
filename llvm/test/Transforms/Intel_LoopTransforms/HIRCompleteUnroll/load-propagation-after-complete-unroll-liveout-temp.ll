; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that first definitions of liveout temp %ld is propagated into the
; single use after complete unroll but second definition is retained.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 199, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 1, 1   <DO_LOOP> <unroll>
; CHECK: |   |   %ld = (%B)[i2];
; CHECK: |   |   (%A)[i2] = %ld;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 199, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = (%B)[0];
; CHECK: |   %ld = (%B)[1];
; CHECK: |   (%A)[1] = %ld;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %A, i32* %B) {
entry:
  br label %outer.loop

outer.loop:
  %iv.outer = phi i64 [ 0, %entry], [ %iv.outer.inc, %outer.latch]
  br label %loop

loop:
  %iv = phi i64 [ 0, %outer.loop], [ %iv.inc, %loop]
  %gepb = getelementptr inbounds i32, i32* %B, i64 %iv
  %ld = load i32, i32* %gepb, align 4
  %gepa = getelementptr inbounds i32, i32* %A, i64 %iv
  store i32 %ld, i32* %gepa, align 4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 2
  br i1 %cmp, label %outer.latch, label %loop, !llvm.loop !0

outer.latch:
  %ld.lcssa = phi i32 [ %ld, %loop ]
  %iv.outer.inc = add i64 %iv.outer, 1
  %cmp1 = icmp eq i64 %iv.outer.inc, 200
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa1 = phi i32 [ %ld.lcssa, %outer.latch ]
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}

