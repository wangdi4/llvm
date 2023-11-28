; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to %A is eliminated by treating fake ref in lifetime.end
; intrinsic in the outer loop as post-dominating store.


; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK: |
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   (%A)[0][5] = %t1;
; CHECK: |   |   %ld = (%A)[0][5];
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @llvm.lifetime.end.p0(40,  &((%A)[0]));
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK: |
; CHECK: |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |   |   %ld = %t1;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @llvm.lifetime.end.p0(40,  &((%A)[0]));
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t1, i32 %t2, i32 %t3) {
entry:
  %A = alloca [10 x i32], align 16
  %gep5 = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 5
  br label %outer.loop

outer.loop:
  %iv.outer = phi i32 [ 0, %entry], [ %iv.outer.inc, %latch]
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %A)
  br label %loop

loop:
  %iv = phi i32 [ 0, %outer.loop], [ %iv.inc, %loop]
  store i32 %t1, ptr %gep5, align 4
  %ld = load i32, ptr %gep5, align 4
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %latch, label %loop

latch:
  call void @llvm.lifetime.end.p0(i64 40, ptr nonnull %A)
  %iv.outer.inc = add i32 %iv.outer, 1
  %cmp1 = icmp eq i32 %iv.outer.inc, 5
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi i32 [ %ld, %latch ]
  ret i32 %ld.lcssa
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #1 = { argmemonly nounwind }
