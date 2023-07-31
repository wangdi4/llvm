; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-sum-window-reuse,print<hir>" -aa-pipeline="basic-aa" < %s -disable-output 2>&1 | FileCheck %s

; This test checks that HIRSumWindowReuse is able to optimize a simple
; multidimensional sliding window sum where the loop bounds are not constants
; and the loops have ZTTs.

; However, this pass does not currently support the style of reduction
; preheader/postexit assignments used in HIR when there are ZTTs present on
; inner sum loops. This isn't an issue for our application of interest because
; the ZTTs are hoisted there, so this test also side-steps the problem by
; hoisting its innermost ZTT.

; Print before:

; CHECK: BEGIN REGION
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   %sum.final = 0.000000e+00;
; CHECK:       |
; CHECK:       |      %sum = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   + DO i3 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |   |   %sum = %sum  +  (%A)[i1 + %stride * i2 + i3];
; CHECK:       |   |   + END LOOP
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
; CHECK:       |
; CHECK:       |      %sum = 0.000000e+00;
; CHECK:       |   + DO i2 = 0, %M + -1, 1   <DO_LOOP>
; CHECK:       |   |   if (i1 == 0)
; CHECK:       |   |   {
; CHECK:       |   |      + DO i3 = 0, %K + -1, 1   <DO_LOOP>
; CHECK:       |   |      |   %[[WSUM]] = %[[WSUM]]  +  (%A)[%stride * i2 + i3];
; CHECK:       |   |      + END LOOP
; CHECK:       |   |   }
; CHECK:       |   |   else
; CHECK:       |   |   {
; CHECK:       |   |      %[[WSUM]] = %[[WSUM]]  -  (%A)[i1 + %stride * i2 + -1];
; CHECK:       |   |      %[[WSUM]] = %[[WSUM]]  +  (%A)[i1 + %stride * i2 + %K + -1];
; CHECK:       |   |   }
; CHECK:       |   + END LOOP
; CHECK:       |      %sum = %sum + %[[WSUM]]
; CHECK:       |      %sum.final = %sum;
; CHECK:       |
; CHECK:       |   (%B)[i1] = %sum.final;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @window2d(ptr %A, ptr noalias %B, i64 %N, i64 %M, i64 %K, i64 %stride) {

entry:
  %ztt.L1 = icmp ne i64 %N, 0
  br i1 %ztt.L1, label %L1.pre, label %exit

L1.pre:
  br label %L1

L1:
  %i = phi i64 [ 0, %L1.pre ], [ %i.next, %L1.latch ]
  %ztt.L2 = icmp ne i64 %M, 0
  %ztt.L3 = icmp ne i64 %K, 0
  %ztt.L2L3 = and i1 %ztt.L2, %ztt.L3
  br i1 %ztt.L2L3, label %L2.pre, label %L1.latch

L2.pre:
  br label %L2

L2:
  %j = phi i64 [ 0, %L2.pre ], [ %j.next, %L2.latch ]
  %sum = phi double [ 0.0, %L2.pre ], [ %sum.next, %L2.latch ]
  %js = mul nsw nuw i64 %j, %stride
  %ij = add nsw nuw i64 %i, %js
  br label %L3

L3:
  %k = phi i64 [ 0, %L2 ], [ %k.next, %L3 ]
  %sum.L3 = phi double [ %sum, %L2 ], [ %sum.next.L3, %L3 ]
  %ijk = add nsw nuw i64 %ij, %k
  %Aijkptr = getelementptr inbounds double, ptr %A, i64 %ijk
  %Aijk = load double, ptr %Aijkptr, align 8
  %sum.next.L3 = fadd fast double %sum.L3, %Aijk
  %k.next = add nsw nuw i64 %k, 1
  %cond.L3 = icmp ne i64 %k.next, %K
  br i1 %cond.L3, label %L3, label %L2.latch

L2.latch:
  %sum.next = phi double [ %sum.next.L3, %L3 ]
  %j.next = add nsw nuw i64 %j, 1
  %cond.L2 = icmp ne i64 %j.next, %M
  br i1 %cond.L2, label %L2, label %L2.exit

L2.exit:
  %sum.lcssa.L2 = phi double [ %sum.next, %L2.latch ]
  br label %L1.latch

L1.latch:
  %sum.final = phi double [ 0.0, %L1 ], [ %sum.lcssa.L2, %L2.exit ]
  %Biptr = getelementptr inbounds double, ptr %B, i64 %i
  store double %sum.final, ptr %Biptr, align 8
  %i.next = add nsw nuw i64 %i, 1
  %cond.L1 = icmp ne i64 %i.next, %N
  br i1 %cond.L1, label %L1, label %L1.exit

L1.exit:
  br label %exit

exit:
  ret void
}
