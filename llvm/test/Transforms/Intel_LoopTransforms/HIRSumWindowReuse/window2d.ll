; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple
; multidimensional sliding window sum.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK:       |   %sum = 0.000000e+00;
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, 7, 1   <DO_LOOP>
; CHECK:       |   |   |   %sum = %sum  +  (%A)[i1 + 64 * i2 + i3];
; CHECK:       |   |   + END LOOP
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
; CHECK:       |
; CHECK:       |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:       |   |   if (i1 == 0)
; CHECK:       |   |   {
; CHECK:       |   |      + DO i3 = 0, 7, 1   <DO_LOOP>
; CHECK:       |   |      |   %[[WSUM]] = %[[WSUM]]  +  (%A)[64 * i2 + i3];
; CHECK:       |   |      + END LOOP
; CHECK:       |   |   }
; CHECK:       |   |   else
; CHECK:       |   |   {
; CHECK:       |   |      %[[WSUM]] = %[[WSUM]]  -  (%A)[i1 + 64 * i2 + -1];
; CHECK:       |   |      %[[WSUM]] = %[[WSUM]]  +  (%A)[i1 + 64 * i2 + 7];
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |      %sum = %sum  +  %[[WSUM]];
; CHECK:       |
; CHECK:       |   (%B)[i1] = %sum;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window2d(ptr %A, ptr noalias %B) {

entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1.latch ]
  br label %L2

L2:
  %j = phi i64 [ 0, %L1 ], [ %j.next, %L2.latch ]
  %sum = phi double [ 0.0, %L1 ], [ %sum.next, %L2.latch ]
  %j64 = mul nsw nuw i64 %j, 64
  %ij = add nsw nuw i64 %i, %j64
  br label %L3

L3:
  %k = phi i64 [ 0, %L2 ], [ %k.next, %L3 ]
  %sum.L3 = phi double [ %sum, %L2 ], [ %sum.next.L3, %L3 ]
  %ijk = add nsw nuw i64 %ij, %k
  %Aijkptr = getelementptr inbounds double, ptr %A, i64 %ijk
  %Aijk = load double, ptr %Aijkptr, align 8
  %sum.next.L3 = fadd fast double %sum.L3, %Aijk
  %k.next = add nsw nuw i64 %k, 1
  %cond.L3 = icmp ne i64 %k.next, 8
  br i1 %cond.L3, label %L3, label %L2.latch

L2.latch:
  %sum.next = phi double [ %sum.next.L3, %L3 ]
  %j.next = add nsw nuw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, 16
  br i1 %cond.L2, label %L2, label %L1.latch

L1.latch:
  %sum.final = phi double [ %sum.next, %L2.latch ]
  %Biptr = getelementptr inbounds double, ptr %B, i64 %i
  store double %sum.final, ptr %Biptr, align 8
  %i.next = add nsw nuw i64 %i, 1
  %cond.L1 = icmp ne i64 %i.next, 56
  br i1 %cond.L1, label %L1, label %exit

exit:
  ret void
}
