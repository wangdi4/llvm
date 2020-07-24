; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -print-before=hir-sum-window-reuse -print-after=hir-sum-window-reuse -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple sliding
; window sum where the loop bounds are not constants and the loops have ZTTs.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   %sum.final = 0.000000e+00;
; CHECK:       |
; CHECK:       |      %sum = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   %sum = %sum  +  (%A)[i1 + i2];
; CHECK:       |   + END LOOP
; CHECK:       |      %sum.final = %sum;
; CHECK:       |
; CHECK:       |   (%B)[i1] = %sum.final;
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:          %[[WSUM:[A-Za-z0-9_.]+]] = 0.000000e+00;
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   %sum.final = 0.000000e+00;
; CHECK:       |   if (%M != 0)
; CHECK:       |   {
; CHECK:       |      %sum = 0.000000e+00;
; CHECK:       |      if (i1 == 0)
; CHECK:       |      {
; CHECK:       |         + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |         |   %[[WSUM]] = %[[WSUM]]  +  (%A)[i2];
; CHECK:       |         + END LOOP
; CHECK:       |      }
; CHECK:       |      else
; CHECK:       |      {
; CHECK:       |         %[[WSUM]] = %[[WSUM]]  -  (%A)[i1 + -1];
; CHECK:       |         %[[WSUM]] = %[[WSUM]]  +  (%A)[i1 + %M + -1];
; CHECK:       |      }
; CHECK:       |      %sum = %sum  +  %[[WSUM]]
; CHECK:       |      %sum.final = %sum;
; CHECK:       |   }
; CHECK:       |   (%B)[i1] = %sum.final;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window1d(double* %A, double*noalias %B, i64 %N, i64 %M) {

entry:
  %ztt.L1 = icmp ne i64 %N, 0
  br i1 %ztt.L1, label %L1.pre, label %exit

L1.pre:
  br label %L1

L1:
  %i = phi i64 [ 0, %L1.pre ], [ %i.next, %L1.latch ]
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
  %cond.L1 = icmp ne i64 %i.next, %N
  br i1 %cond.L1, label %L1, label %L1.exit

L1.exit:
  br label %exit

exit:
  ret void
}
