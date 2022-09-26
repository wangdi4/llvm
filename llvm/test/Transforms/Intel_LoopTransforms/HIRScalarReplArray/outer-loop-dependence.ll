; RUN: opt -passes="hir-ssa-deconstruction,hir-scalarrepl-array" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to scalar replace this load group:
; {(@A)[0][i1][i2], (@A)[0][i1][i2 + 1]}

; We should ignore dependence with the store (@A)[0][i1 + 1][i2]
; because the dependence is carried by the outer loop.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK: |   |   %0 = (@A)[0][i1][i2];
; CHECK: |   |   %1 = (@A)[0][i1][i2 + 1];
; CHECK: |   |   (@A)[0][i1 + 1][i2] = %0 + %1;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION


; CHECK: Dump After

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |      %scalarepl = (@A)[0][i1][0];
; CHECK: |   + DO i2 = 0, 98, 1   <DO_LOOP>
; CHECK: |   |   %0 = %scalarepl;
; CHECK: |   |   %scalarepl2 = (@A)[0][i1][i2 + 1];
; CHECK: |   |   %1 = %scalarepl2;
; CHECK: |   |   (@A)[0][i1 + 1][i2] = %0 + %1;
; CHECK: |   |   %scalarepl = %scalarepl2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc16
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.inc16 ]
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv31, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv31, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx9, align 4
  %add10 = add nsw i32 %1, %0
  %arrayidx15 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv.next32, i64 %indvars.iv
  store i32 %add10, i32* %arrayidx15, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, 99
  br i1 %exitcond.not, label %for.inc16, label %for.body3

for.inc16:                                        ; preds = %for.body3
  %exitcond33.not = icmp eq i64 %indvars.iv.next32, 99
  br i1 %exitcond33.not, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.inc16
  ret void
}

