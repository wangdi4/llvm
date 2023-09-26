; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to (%A)[0][0][i1] and (%A)[0][1][i1] are eliminated by
; propagating to the corresponding loads. %A is identified as a region local
; alloca. The two locations are identified as distinct so the intermediate
; loads are not perceived as intervening refs.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (%A)[0][0][i1] = %t1;
; CHECK: |   (%A)[0][1][i1] = %t2;
; CHECK: |   %ld1 = (%A)[0][0][i1];
; CHECK: |   %ld2 = (%A)[0][1][i1];
; CHECK: |   %add = %ld1  +  %ld2;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %add = %t1  +  %t2;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t1, i32 %t2) {
entry:
  %A = alloca [2 x [10 x i32]], align 16
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %loop]
  %iv.inc = add i64 %iv, 1
  %gep = getelementptr inbounds [2 x [10 x i32]], ptr %A, i64 0, i64 0, i64 %iv
  %gep1 = getelementptr inbounds [2 x [10 x i32]], ptr %A, i64 0, i64 1, i64 %iv
  store i32 %t1, ptr %gep, align 4
  store i32 %t2, ptr %gep1, align 4
  %ld1 = load i32, ptr %gep, align 4
  %ld2 = load i32, ptr %gep1, align 4
  %add = add i32 %ld1, %ld2
  %cmp = icmp eq i64 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  %add.lcssa = phi i32 [ %add, %loop ]
  ret i32 %add.lcssa
}


