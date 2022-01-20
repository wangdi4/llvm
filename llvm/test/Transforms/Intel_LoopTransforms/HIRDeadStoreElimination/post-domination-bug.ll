; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that the store (@A)[0][i1] = 0 is not incorrectly eliminated due to a bug in post-domination logic.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = 0;
; CHECK: + END LOOP

; CHECK: (@B)[0][2] = 10;

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   switch(%t)
; CHECK: |   {
; CHECK: |   case 0:
; CHECK: |      (@B)[0][0] = 5;
; CHECK: |      break;
; CHECK: |   case 1:
; CHECK: |      (@B)[0][1] = 2;
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      goto L;
; CHECK: |   }
; CHECK: |   (@A)[0][i1] = 5;
; CHECK: |   L:
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: (@A)[0][i1] = 0;

; CHECK: (@A)[0][i1] = 5;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [5 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [5 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %t) {
entry:
  br label %for.body

for.cond1.preheader:                              ; preds = %for.body
  store i32 10, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @B, i64 0, i64 2), align 8
  br label %for.body3

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.body ]
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* @A, i64 0, i64 %indvars.iv20
  store i32 0, i32* %arrayidx, align 4
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond23.not = icmp eq i64 %indvars.iv.next21, 5
  br i1 %exitcond23.not, label %for.cond1.preheader, label %for.body

for.body3:                                        ; preds = %for.cond1.preheader, %L
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %L ]
  switch i32 %t, label %L [
    i32 0, label %sw.bb
    i32 1, label %sw.bb4
  ]

sw.bb:                                            ; preds = %for.body3
  store i32 5, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @B, i64 0, i64 0), align 16
  br label %sw.epilog

sw.bb4:                                           ; preds = %for.body3
  store i32 2, i32* getelementptr inbounds ([5 x i32], [5 x i32]* @B, i64 0, i64 1), align 4
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.bb4, %sw.bb
  %arrayidx6 = getelementptr inbounds [5 x i32], [5 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 5, i32* %arrayidx6, align 4
  br label %L

L:                                                ; preds = %for.body3, %sw.epilog
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond.not, label %for.end9, label %for.body3

for.end9:                                         ; preds = %L
  ret void
}

