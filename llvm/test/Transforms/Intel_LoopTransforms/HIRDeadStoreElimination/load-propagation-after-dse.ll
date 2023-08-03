; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that region local alloca load/store to (%A)[0][i1] is replaced by temp
; by DSE and then the temp is propagated to its single use by the utility.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][i1] = (%B)[i1];
; CHECK: |   (%C)[i1] = (%A)[0][i1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%C)[i1] = (%B)[i1];
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %B, ptr %C) {
entry:
  %A = alloca [10 x i32], align 16
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %iv.inc = add i64 %iv, 1
  %gepb = getelementptr inbounds i32, ptr %B, i64 %iv
  %ld1 = load i32, ptr %gepb, align 4
  %gepa = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  store i32 %ld1, ptr %gepa, align 4
  %ld2 = load i32, ptr %gepa, align 4
  %gepc = getelementptr inbounds i32, ptr %C, i64 %iv
  store i32 %ld2, ptr %gepc, align 4
  %cmp = icmp eq i64 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}


