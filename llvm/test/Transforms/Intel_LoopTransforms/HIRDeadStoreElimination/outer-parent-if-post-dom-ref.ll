; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that store to %A in i2 loop is eliminated using the post-dominating
; ref in outer loop when the i2 loop is within an if..


; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %ld.merge = 0;
; CHECK: |   if (%n > 0)
; CHECK: |   {
; CHECK: |      + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |      |   (%A)[0][5] = %t1;
; CHECK: |      |   %ld = (%A)[0][5];
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %ld.merge = %ld;
; CHECK: |   }
; CHECK: |   (%A)[0][5] = 10;
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %ld.merge = 0;
; CHECK: |   if (%n > 0)
; CHECK: |   {
; CHECK: |      + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK: |      |   %ld = %t1;
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %ld.merge = %ld;
; CHECK: |   }
; CHECK: |   (%A)[0][5] = 10;
; CHECK: + END LOOP



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo([10 x i32]* %A, i32 %t1, i32 %t2, i32 %t3, i32 %n) {
entry:
  %gep5 = getelementptr inbounds [10 x i32], [10 x i32]* %A, i64 0, i64 5
  br label %outer.loop

outer.loop:
  %iv.outer = phi i32 [ 0, %entry], [ %iv.outer.inc, %latch]
  %cmpn = icmp sgt i32 %n, 0
  br i1 %cmpn, label %loop.pre, label %latch

loop.pre:
  br label %loop

loop:
  %iv = phi i32 [ 0, %loop.pre], [ %iv.inc, %loop]
  store i32 %t1, i32* %gep5, align 4
  %ld = load i32, i32* %gep5, align 4
  %iv.inc = add i32 %iv, 1
  %cmp = icmp eq i32 %iv.inc, 5
  br i1 %cmp, label %loopexit, label %loop

loopexit:
  br label %latch

latch:
  %ld.merge = phi i32 [ 0, %outer.loop ], [ %ld, %loopexit ]
  store i32 10, i32* %gep5, align 4
  %iv.outer.inc = add i32 %iv.outer, 1
  %cmp1 = icmp eq i32 %iv.outer.inc, 5
  br i1 %cmp1, label %exit, label %outer.loop

exit:
  %ld.lcssa = phi i32 [ %ld.merge, %latch ]
  ret i32 %ld.lcssa
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }
