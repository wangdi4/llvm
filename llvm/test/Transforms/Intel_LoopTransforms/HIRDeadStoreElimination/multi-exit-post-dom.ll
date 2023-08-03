; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that the second store to A[2] is deduced as post-dominating the first
; store even though its parent is a multi-exit loop and the first store is
; eliminated.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 50, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK: |   |   (%A)[2] = 5;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   (%A)[2] = 10;
; CHECK: |   |   %ldB = (%B)[i2];
; CHECK: |   |   if (%ldB == 3)
; CHECK: |   |   {
; CHECK: |   |      goto for.outer.latch;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   for.outer.latch:
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK:     BEGIN REGION { modified }
; CHECK-NEXT: + DO i1 = 0, 50, 1   <DO_LOOP>
; CHECK-NEXT: |   + DO i2 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
; CHECK-NEXT: |   |   (%A)[2] = 10;
; CHECK-NEXT: |   |   %ldB = (%B)[i2];
; CHECK-NEXT: |   |   if (%ldB == 3)
; CHECK-NEXT: |   |   {
; CHECK-NEXT: |   |      goto for.outer.latch;
; CHECK-NEXT: |   |   }
; CHECK-NEXT: |   + END LOOP
; CHECK-NEXT: |
; CHECK-NEXT: |   for.outer.latch:
; CHECK-NEXT: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noalias %A, ptr noalias %B) {
entry:
  br label %for.body.outer

for.body.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %iv.outer.next, %for.outer.latch ]
  br label %for.body1

for.body1:                                         ; preds = %entry, %for.latch
  %indvars.iv1 = phi i64 [ 0, %for.body.outer ], [ %indvars.iv.next1, %for.body1 ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 2
  store i32 5, ptr %arrayidx, align 4
  %indvars.iv.next1 = add nuw nsw i64 %indvars.iv1, 1
  %cond = icmp eq i64 %indvars.iv.next1, 50
  br i1 %cond, label %for.exit, label %for.body1

for.exit:
  br label %for.body2

for.body2:                                         ; preds = %entry, %for.latch
  %indvars.iv2 = phi i64 [ 0, %for.exit ], [ %indvars.iv.next2, %for.latch ]
  store i32 10, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv2
  %ldB = load i32, ptr %arrayidx1, align 4
  %cmp5 = icmp eq i32 %ldB, 3
  br i1 %cmp5, label %for.outer.latch, label %for.latch

for.latch:                                         ; preds = %for.body
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next2, 50
  br i1 %exitcond.not, label %for.outer.latch, label %for.body2

for.outer.latch:
  %iv.outer.next = add nuw nsw i64 %iv.outer, 1
  %exit.cond = icmp eq i64 %iv.outer, 50
  br i1 %exit.cond, label %exit, label %for.body.outer
  
exit:                                          ; preds = %for.body, %for.latch
  ret void
}

