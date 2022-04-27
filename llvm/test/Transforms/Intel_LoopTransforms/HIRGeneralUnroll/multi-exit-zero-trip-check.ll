; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s

; The trip count of this loop can be 2^32 (interpreted as 0) when %N is equal
; to 1. Depending on the value of %K, it can take the early exit in 1st, 2nd,
; 3rd iteration etc without causing undefined behavior by violating no wrap
; semantics.
; To handle this case we add a zero trip check on the remainder loop to make
; it run all the iterations.
; Note that LEGAL_MAX_TC/MAX_TC_EST is not set on the remainder loop.

; Input HIR-
; + DO i1 = 0, %N + -2, 1   <DO_MULTI_EXIT_LOOP>
; |   %iv.out = -1 * i1 + %N;
; |   if (i1 + -1 * %N + %K == -4)
; |   {
; |      goto exit;
; |   }
; |   (@glob)[0] = -1 * i1 + %N + -1;
; + END LOOP

; CHECK: + DO i1 = 0, %tgu + -1, 1    <DO_MULTI_EXIT_LOOP> <nounroll>

; CHECK: %bound.check = 2 * %tgu <u %N + -1;
; CHECK: %zero.tc.check = %N + -1 == 0;
; CHECK: %combined.ztt = %bound.check  |  %zero.tc.check;
; CHECK: if (%combined.ztt != 0)
; CHECK: {
; CHECK: + DO i1 = 2 * %tgu, %N + -2, 1   <DO_MULTI_EXIT_LOOP> <nounroll>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@glob = external global i32, align 8

define void @eggs(i32 %K, i32 %N) {
entry:
  br label %loop

loop:                                             ; preds = %latch, %entry
  %iv = phi i32 [ %iv.inc, %latch ], [ %N, %entry ]
  %tmp36 = sub i32 %K, %iv
  %tmp37 = icmp eq i32 %tmp36, -4
  br i1 %tmp37, label %exit, label %latch

latch:                                             ; preds = %loop
  %iv.inc = add nsw i32 %iv, -1
  store i32 %iv.inc, i32* @glob, align 8
  %tmp40 = icmp eq i32 %iv.inc, 1
  br i1 %tmp40, label %exit, label %loop

exit:                                             ; preds = %latch, %loop
  %tmp42 = phi i32 [ 1, %latch ], [ %iv, %loop ]
  ret void
}
