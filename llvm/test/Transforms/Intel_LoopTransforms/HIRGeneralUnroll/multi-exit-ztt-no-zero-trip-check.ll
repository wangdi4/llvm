; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s

; Check that we can deduce that the trip count of this loop cannot be 2^32 due to
; ztt in incoming IR so we are able to avoid emitting zero trip count check and
; completely unroll remainder loop of trip count 1.

; Input HIR-
; + DO i1 = 0, %N + -2, 1   <DO_MULTI_EXIT_LOOP>
; |   %iv.out = -1 * i1 + %N;
; |   if (i1 + -1 * %N + %K == -4)
; |   {
; |      goto exit;
; |   }
; |   (@glob)[0] = -1 * i1 + %N + -1;
; + END LOOP

; CHECK: + DO i1 = 0, %tgu + -1, 1    <DO_MULTI_EXIT_LOOP> 

; CHECK: if (2 * %tgu <u %N + -1)
; CHECK: {
; CHECK-NOT: DO i1


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@glob = external global i32, align 8

define void @eggs(i32 %K, i32 %N) {
entry:
  %ztt = icmp sgt i32 %N, 1
  br i1 %ztt, label %loop.pre, label %exit

loop.pre:
  br label %loop

loop:                                             ; preds = %latch, %loop.pre
  %iv = phi i32 [ %iv.inc, %latch ], [ %N, %loop.pre ]
  %tmp36 = sub i32 %K, %iv
  %tmp37 = icmp eq i32 %tmp36, -4
  br i1 %tmp37, label %loopexit, label %latch

latch:                                             ; preds = %loop
  %iv.inc = add nsw i32 %iv, -1
  store i32 %iv.inc, i32* @glob, align 8
  %tmp40 = icmp eq i32 %iv.inc, 1
  br i1 %tmp40, label %loopexit, label %loop

loopexit:                                             ; preds = %latch, %loop
  %tmp42 = phi i32 [ 1, %latch ], [ %iv, %loop ]
  br label %exit

exit:
  ret void
}
