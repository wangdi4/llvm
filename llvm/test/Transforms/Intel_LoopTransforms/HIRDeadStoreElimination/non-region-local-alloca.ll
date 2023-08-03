; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to %A is not eliminated because %A alloca has a use outside
; the region in %ld2.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][5] = %t;
; CHECK: |   %ld1 = (%A)[0][5];
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][5] = %t;
; CHECK: |   %ld1 = (%A)[0][5];
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %bc = bitcast ptr %A to ptr
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 5
  %A0 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 0
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry], [ %iv.inc, %loop]
  store i32 %t, ptr %gep, align 4
  %ld1 = load i32, ptr %gep, align 4
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %ld1, %loop ]
  %ld2 = load i32, ptr %gep, align 4
  %add = add i32 %ld.lcssa, %ld2
  ret i32 %add
}


