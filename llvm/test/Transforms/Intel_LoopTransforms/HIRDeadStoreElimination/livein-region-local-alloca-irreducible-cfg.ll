; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store and 2nd load of (%A)[0][0] are not eliminated even though %A
; is identified as a region local alloca because the region can be re-entered
; due to loop being inside an irreducible graph cycle with entry blocks
; %irred.entry1 and %irred.entry2 in LLVM IR and there is a prior load of
; (%A)[0][0] which can reuse the value from store.

; Incoming HIR-
; + DO i1 = 0, 0, 1   <DO_LOOP> <unroll>
; |   %ld = (%A)[0][i1];
; |   (%A)[0][i1] = %ld + %t;
; |   %ld1 = (%A)[0][i1];
; + END LOOP

; CHECK: Dump Before

; CHECK: BEGIN REGION { modified }
; CHECK: %ld = (%A)[0][0];
; CHECK: (%A)[0][0] = %ld + %t;
; CHECK: %ld1 = (%A)[0][0];
; CHECK: END REGION

; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: %ld = (%A)[0][0];
; CHECK: (%A)[0][0] = %ld + %t;
; CHECK: %ld1 = (%A)[0][0];
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %t, i1 %cond1, i1 %cond2) {
entry:
  %A = alloca [10 x i32], align 16
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 16 %A, i8 0, i64 40, i1 false)
  br i1 %cond1, label %irred.entry1, label %irred.entry2

irred.entry1:
  br label %loop.pre

loop.pre:
  br label %loop

loop:
  %iv = phi i64 [ 0, %loop.pre], [ %iv.inc, %loop]
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  %ld = load i32, ptr %gep, align 4
  %add = add i32 %ld, %t
  store i32 %add, ptr %gep, align 4
  %ld1 = load i32, ptr %gep, align 4
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 1
  br i1 %cmp, label %loop.exit, label %loop, !llvm.loop !0

loop.exit:
  %ld.lcssa = phi i32 [ %ld1, %loop ]
  br label %irred.entry2

irred.entry2:
  br i1 %cond2, label %exit, label %irred.entry1

exit:
  ret void
}

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) 

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.unroll.full"}

