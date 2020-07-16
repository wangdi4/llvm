; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -print-before=hir-sum-window-reuse -debug-only=hir-sum-window-reuse -print-after=hir-sum-window-reuse -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-sum-window-reuse < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple window
; sliding window sum in an extreme loop structure where the outer loop is
; an unknown loop and the inner loop's bounds vary outside of the outer loop.

; Currently, this transform is not yet implemented and so for now this test just
; checks that the initial analysis correctly identifies the sum for
; optimization.

; REQUIRES: asserts

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK:       |   %M = (%Ms)[i1];
; CHECK:       |
; CHECK:       |   + UNKNOWN LOOP i2
; CHECK:       |   |   <i2 = 0>
; CHECK:       |   |   L1:
; CHECK:       |   |   %sum.final = 0.000000e+00;
; CHECK:       |   |
; CHECK:       |   |      %sum = 0.000000e+00;
; CHECK:       |   |   + DO i3 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   %sum = %sum  +  (%A)[i2 + i3];
; CHECK:       |   |   + END LOOP
; CHECK:       |   |      %sum.final = %sum;
; CHECK:       |   |
; CHECK:       |   |   (%B)[i2] = %sum.final;
; CHECK:       |   |   if ((%OuterStop)[i2] == 0)
; CHECK:       |   |   {
; CHECK:       |   |      <i2 = i2 + 1>
; CHECK:       |   |      goto L1;
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP

; CHECK:       ret ;
; CHECK: END REGION

; Debug output:

; CHECK: Identified these window sums for optimization:
; CHECK:   %sum = %sum  +  (%A)[i2 + i3];

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window1d(double* %A, double*noalias %B, i8* %OuterStop, i64* %Ms) {

entry:
  br label %L0

L0:
  %h = phi i64 [ 0, %entry ], [ %h.next, %L0.latch ]
  %Mptr = getelementptr inbounds i64, i64* %Ms, i64 %h
  %M = load i64, i64* %Mptr, align 8
  br label %L1

L1:
  %i = phi i64 [ 0, %L0 ], [ %i.next, %L1.latch ]
  %ztt.L2 = icmp ne i64 %M, 0
  br i1 %ztt.L2, label %L2.pre, label %L1.latch

L2.pre:
  br label %L2

L2:
  %j = phi i64 [ 0, %L2.pre ], [ %j.next, %L2 ]
  %sum = phi double [ 0.0, %L2.pre ], [ %sum.next, %L2 ]
  %ij = add nuw nsw i64 %i, %j
  %Aijptr = getelementptr inbounds double, double* %A, i64 %ij
  %Aij = load double, double* %Aijptr, align 8
  %sum.next = fadd fast double %sum, %Aij
  %j.next = add nuw nsw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, %M
  br i1 %cond.L2, label %L2, label %L2.exit

L2.exit:
  %sum.lcssa = phi double [ %sum.next, %L2 ]
  br label %L1.latch

L1.latch:
  %sum.final = phi double [ 0.0, %L1 ], [ %sum.lcssa, %L2.exit ]
  %Biptr = getelementptr inbounds double, double* %B, i64 %i
  store double %sum.final, double* %Biptr, align 8
  %i.next = add nuw nsw i64 %i, 1
  %stopptr = getelementptr inbounds i8, i8* %OuterStop, i64 %i
  %stop = load i8, i8* %stopptr
  %cond.L1 = icmp eq i8 %stop, 0
  br i1 %cond.L1, label %L1, label %L0.latch

L0.latch:
  %h.next = add nuw nsw i64 %h, 1
  %cond.L0 = icmp ne i64 %h.next, 64
  br i1 %cond.L0, label %L0, label %exit

exit:
  ret void
}
