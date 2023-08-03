; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 -disable-output < %s | FileCheck %s

; Verify that we are able to conclude that the first store to A[2] dominates the
; load: %ld = A[2] despite presence of goto/label in between them and the 
; store is eliminated by propagating its rval to the load.

; CHECK: Dump Before

; CHECK: BEGIN REGION
; CHECK: (%A)[2] = 5;

; CHECK: + DO i1 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %ldB = (%B)[i1];
; CHECK: |   if (%ldB == 3)
; CHECK: |   {
; CHECK: |      goto for.end;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: for.end:
; CHECK: %ld = (%A)[2];
; CHECK: (%A)[2] = %ld + 5;


; CHECK: Dump After

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT: + DO i1 = 0, 49, 1   <DO_MULTI_EXIT_LOOP>
; CHECK-NEXT: |   %ldB = (%B)[i1];
; CHECK-NEXT: |   if (%ldB == 3)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      goto for.end;
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP

; CHECK: for.end:
; CHECK-NEXT: (%A)[2] = 10;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noalias %A, ptr noalias %B) {
entry:
  br label %bb

bb:
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 2
  store i32 5, ptr %arrayidx, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %for.cond
  %indvars.iv = phi i64 [ 0, %bb ], [ %indvars.iv.next, %for.cond ]
  %arrayidx1 = getelementptr inbounds i32, ptr %B, i64 %indvars.iv
  %ldB = load i32, ptr %arrayidx1, align 4
  %cmp5 = icmp eq i32 %ldB, 3
  br i1 %cmp5, label %for.end, label %for.cond

for.cond:                                         ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %for.cond
  %ld = load i32, ptr %arrayidx, align 4
  %add = add i32 %ld, 5
  store i32 %add, ptr %arrayidx, align 4
  ret void
}

