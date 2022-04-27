; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -print-before=hir-sum-window-reuse -print-after=hir-sum-window-reuse -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -print-before=hir-sum-window-reuse -print-after=hir-sum-window-reuse -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -opaque-pointers -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple sliding
; window sum.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %sum = 0.000000e+00;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:       |   |   %sum = %sum  +  (%A)[i1 + i2];
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   (%B)[i1] = %sum;
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:          %[[WSUM:[A-Za-z0-9_.]+]] = 0.000000e+00;
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %sum = 0.000000e+00;
; CHECK:       |   if (i1 == 0)
; CHECK:       |   {
; CHECK:       |      + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:       |      |   %[[WSUM]] = %[[WSUM]]  +  (%A)[i2];
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %[[WSUM]] = %[[WSUM]]  -  (%A)[i1 + -1];
; CHECK:       |      %[[WSUM]] = %[[WSUM]]  +  (%A)[i1 + 7];
; CHECK:       |   }
; CHECK:       |   %sum = %sum  +  %[[WSUM]]
; CHECK:       |   (%B)[i1] = %sum;
; CHECK:       + END LOOP
; CHECK: END REGION

; Also check the opt report functionality, which is only enabled for internal
; builds:

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sum-window-reuse -hir-cg -simplifycfg -intel-ir-optreport-emitter -intel-opt-report=low -disable-output 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; REQUIRES: intel_internal_build

; Opt report:

; OPTREPORT: LOOP BEGIN
; OPTREPORT:     remark #25584: Inner loop sums optimized with sum window reuse

; OPTREPORT:     LOOP BEGIN
; OPTREPORT:     <Window sum initialization loop for sum window reuse>
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window1d(double* %A, double*noalias %B) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2 ]
  %sum = phi double [ 0.0, %L1 ], [ %sum.next, %L2 ]
  %ij = add nsw nuw i64 %i, %j
  %Aijptr = getelementptr inbounds double, double* %A, i64 %ij
  %Aij = load double, double* %Aijptr, align 8
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
