; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to region local alloca %A is eliminated as dead store and
; loop with lifetime intrinsics is eliminated by redundant node removal.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   @llvm.lifetime.start.p0(40,  &((%A)[0]));
; CHECK: |   (%A)[0][5] = %t;
; CHECK: |   @llvm.lifetime.end.p0(40,  &((%A)[0]));
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %gep = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 5
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry], [ %iv.inc, %loop]
  call void @llvm.lifetime.start.p0(i64 40, ptr nonnull %A)
  store i32 %t, ptr %gep, align 4
  call void @llvm.lifetime.end.p0(i64 40, ptr nonnull %A)
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #1 = { argmemonly nounwind }

