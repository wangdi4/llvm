; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a mulitple sliding
; window sums within a single loop.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %Asum = 0.000000e+00;
; CHECK:       |   %Bsum = 0.000000e+00;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:       |   |   %Asum = %Asum  +  (%A)[i1 + i2];
; CHECK:       |   |   %Bsum = %Bsum  -  (%B)[i1 + i2];
; CHECK:       |   + END LOOP
; CHECK:       |
; CHECK:       |   %prod = %Asum * %Bsum;
; CHECK:       |   (%C)[i1] = %prod;
; CHECK:       + END LOOP
; CHECK: END REGION

; Print after:

; CHECK: BEGIN REGION
; CHECK:          %[[AWSUM:[A-Za-z0-9_.]+]] = 0.000000e+00;
; CHECK:          %[[BWSUM:[A-Za-z0-9_.]+]] = 0.000000e+00;
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %Asum = 0.000000e+00;
; CHECK:       |   %Bsum = 0.000000e+00;
; CHECK:       |   if (i1 == 0)
; CHECK:       |   {
; CHECK:       |      + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:       |      |   %[[AWSUM]] = %[[AWSUM]]  +  (%A)[i2];
; CHECK:       |      |   %[[BWSUM]] = %[[BWSUM]]  -  (%B)[i2];
; CHECK:       |      + END LOOP
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %[[AWSUM]] = %[[AWSUM]]  -  (%A)[i1 + -1];
; CHECK:       |      %[[AWSUM]] = %[[AWSUM]]  +  (%A)[i1 + 7];
; CHECK:       |      %[[BWSUM]] = %[[BWSUM]]  +  (%B)[i1 + -1];
; CHECK:       |      %[[BWSUM]] = %[[BWSUM]]  -  (%B)[i1 + 7];
; CHECK:       |   }
; CHECK:       |   %Bsum = %Bsum  +  %[[BWSUM]]
; CHECK:       |   %Asum = %Asum  +  %[[AWSUM]]
; CHECK:       |   %prod = %Asum * %Bsum;
; CHECK:       |   (%C)[i1] = %prod;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window1d(ptr %A, ptr %B, ptr noalias %C) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2 ]
  %Asum = phi double [ 0.0, %L1 ], [ %Asum.next, %L2 ]
  %Bsum = phi double [ 0.0, %L1 ], [ %Bsum.next, %L2 ]
  %ij = add nsw nuw i64 %i, %j
  %Aijptr = getelementptr inbounds double, ptr %A, i64 %ij
  %Aij = load double, ptr %Aijptr, align 8
  %Asum.next = fadd fast double %Asum, %Aij
  %Bijptr = getelementptr inbounds double, ptr %B, i64 %ij
  %Bij = load double, ptr %Bijptr, align 8
  %Bsum.next = fsub fast double %Bsum, %Bij
  %j.next = add nsw nuw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, 8
  br i1 %cond.L2, label %L2, label %L1.latch

L1.latch:
  %Asum.final = phi double [ %Asum.next, %L2 ]
  %Bsum.final = phi double [ %Bsum.next, %L2 ]
  %prod = fmul fast double %Asum.final, %Bsum.final
  %Ciptr = getelementptr inbounds double, ptr %C, i64 %i
  store double %prod, ptr %Ciptr, align 8
  %i.next = add nsw nuw i64 %i, 1
  %cond.L1 = icmp ne i64 %i.next, 56
  br i1 %cond.L1, label %L1, label %exit

exit:
  ret void
}
