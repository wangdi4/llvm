; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that this test case compiles successfully. We are able to eliminate the
; first store in outer loop. We are also able to eliminate the second store in
; the inner loop by subtituting it and the intermediate load with the temp.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   (%ptr)[0] = 0;
; CHECK: |
; CHECK: |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   (%ptr)[0] = 1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %ld = (%ptr)[0];
; CHECK: |   (%ptr)[0] = 2;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   |   %temp = 1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %ld = %temp;
; CHECK: |   (%ptr)[0] = 2;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %ptr) {
entry:
  br label %outer

outer:                                       ; preds = %latch, %entry
  store i32 0, ptr %ptr, align 16
  br label %inner

inner:                                       ; preds = %inner, %outer
  store i32 1, ptr %ptr, align 16
  %cmp35.not = icmp ugt i64 1, 0
  br i1 %cmp35.not, label %latch, label %inner

latch:                               ; preds = %inner
  %ld = load i32, ptr %ptr, align 16
  store i32 2, ptr %ptr, align 16
  %exitcond424.not = icmp eq i64 0, 0
  br i1 %exitcond424.not, label %exit, label %outer

exit:                           ; preds = %latch
  %ld.lcssa = phi i32 [ %ld, %latch ]
  ret void
}
