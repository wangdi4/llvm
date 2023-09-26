; RUN: opt -hir-create-function-level-region  -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we can eliminate store to region local alloca (%A)[5] by
; propagating its value to the load in the sibling loop.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%A)[5] = i1;
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = (%A)[5];
; CHECK: + END LOOP

; CHECK: ret %ld;


; CHECK: Dump After

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %temp = i1;
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %ld = %temp;
; CHECK: + END LOOP

; CHECK: ret %ld;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo() {
entry:
  %A = alloca i32, align 16
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv21 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next22, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 5
  %0 = trunc i64 %indvars.iv21 to i32
  store i32 %0, ptr %arrayidx, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next22, 100
  br i1 %exitcond24, label %for.body3.preheader, label %for.body

for.body3.preheader:                              ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds i32, ptr %A, i64 5
  %ld = load i32, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end8.loopexit, label %for.body3

for.end8.loopexit:                                ; preds = %for.body3
  %ld.lcssa = phi i32 [ %ld, %for.body3]
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit
  %ld.out = phi i32 [ %ld.lcssa, %for.end8.loopexit ]
  ret i32 %ld.out
}

