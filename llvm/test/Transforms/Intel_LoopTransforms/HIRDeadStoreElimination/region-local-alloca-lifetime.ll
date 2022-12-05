; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to %A is eliminated by propagating to the load because
; we know that %A is not dereferenced in @bar.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   @llvm.lifetime.start.p0i8(40,  &((i8*)(%A)[0]));
; CHECK: |   (%A)[0][5] = %t;
; CHECK: |   %ld = (%A)[0][5];
; CHECK: |   @llvm.lifetime.end.p0i8(4000,  &((i8*)(%A)[0]));
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   @llvm.lifetime.start.p0i8(40,  &((i8*)(%A)[0]));
; CHECK: |   %ld = %t;
; CHECK: |   @llvm.lifetime.end.p0i8(4000,  &((i8*)(%A)[0]));
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32 %t) {
entry:
  %A = alloca [10 x i32], align 16
  %bc = bitcast [10 x i32]* %A to i8*
  %gep = getelementptr inbounds [10 x i32], [10 x i32]* %A, i64 0, i64 5
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry], [ %iv.inc, %loop]
  call void @llvm.lifetime.start.p0i8(i64 40, i8* nonnull %bc)
  store i32 %t, i32* %gep, align 4
  %ld = load i32, i32* %gep, align 4
  call void @llvm.lifetime.end.p0i8(i64 4000, i8* nonnull %bc)
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  %ld.lcssa = phi i32 [ %ld, %loop ]
  ret i32 %ld.lcssa
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
