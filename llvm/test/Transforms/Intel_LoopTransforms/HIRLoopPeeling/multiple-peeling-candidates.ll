; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-peeling" -disable-output -print-before=hir-loop-peeling -print-after=hir-loop-peeling 2>&1 < %s  | FileCheck %s

; Verify that we can handle multiple peeling candidates in the same loop.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][0];
; CHECK: |   (@A)[0][i1] = %0 + %t.09;
; CHECK: |   %t.09 = (@B)[0][i1];
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Dump After

; CHECK: %0 = (@A)[0][0];
; CHECK: (@A)[0][0] = %0 + %t.09;

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %t.09 = (@B)[0][i1];
; CHECK: |   %0 = (@A)[0][0];
; CHECK: |   (@A)[0][i1 + 1] = %0 + %t.09;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t.09 = phi i32 [ 5, %entry ], [ %1, %for.body ]
  %0 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @A, i64 0, i64 0), align 16
  %add = add nsw i32 %t.09, %0
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx2, align 4
  %arrayidx = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

