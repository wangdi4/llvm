; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -print-before=hir-sum-window-reuse -debug-only=hir-sum-window-reuse -print-after=hir-sum-window-reuse -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-sum-window-reuse < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple window
; sliding window sum with a term load that involves a cast.

; Currently, this transform is not yet implemented and so for now this test just
; checks that the initial analysis correctly identifies the sum for
; optimization.

; REQUIRES: asserts

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %sum = 0.000000e+00;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:       |   |   %sum = %sum  +  (double*)(%A)[i1 + i2];
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   (%B)[i1] = %sum;
; CHECK:       + END LOOP
; CHECK: END REGION

; Debug output:

; CHECK: Identified these window sums for optimization:
; CHECK:   %sum = %sum  +  (double*)(%A)[i1 + i2];

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window1d(i64* %A, double*noalias %B) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum = phi double [ 0.0, %L1 ], [ %sum.next, %L2 ]
  %ij = add nsw nuw i64 %i, %j
  %Aijptr = getelementptr inbounds i64, i64* %A, i64 %ij
  %Aijdoubleptr = bitcast i64* %Aijptr to double*
  %Aij = load double, double* %Aijdoubleptr, align 8
  %sum.next = fadd fast double %sum, %Aij
  %j.next = add nsw nuw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, 8
  br i1 %cond.L2, label %L2, label %L1.latch

L1.latch:
  %sum.final = phi double [ %sum.next, %L2 ]
  %Biptr = getelementptr inbounds double, double* %B, i64 %i
  store double %sum.final, double* %Biptr, align 8
  %i.next = add nsw nuw i64 %i, 1
  %cond.L1 = icmp ne i64 %i.next, 56
  br i1 %cond.L1, label %L1, label %exit

exit:
  ret void
}
