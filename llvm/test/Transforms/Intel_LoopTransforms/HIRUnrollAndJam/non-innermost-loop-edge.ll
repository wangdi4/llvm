; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam" -debug-only=hir-unroll-and-jam -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we give up on unroll & jam of i1 loop on encountering the edge
; (@A)[0][i1 + 1][i2] --> (@A)[0][i1][i2] FLOW (< =)
; This dependency is carries by i1 loop and the edge exists at the i2 level.
; Non-innermost loop edges like these cannot be correctly handled by unroll
; & jam.

; Input HIR-
; + DO i1 = 0, 98, 1   <DO_LOOP>
; |   + DO i2 = 0, 98, 1   <DO_LOOP>
; |   |   (@B)[0][i1][i2] = (@A)[0][i1][i2];
; |   |
; |   |   + DO i3 = 0, 98, 1   <DO_LOOP>
; |   |   |   %1 = (@C)[0][i2][i3];
; |   |   |   (@C)[0][i2][i3] = %1 + 1;
; |   |   + END LOOP
; |   |
; |   |   (@A)[0][i1 + 1][i2] = (@D)[0][i1][i2];
; |   + END LOOP
; + END LOOP

; CHECK: Illegal edge found: 
; CHECK-SAME: (@A)[0][i1 + 1][i2] --> (@A)[0][i1][i2] FLOW (< =)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc29
  %indvars.iv58 = phi i64 [ 0, %entry ], [ %indvars.iv.next59, %for.inc29 ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.end
  %indvars.iv55 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next56, %for.end ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv58, i64 %indvars.iv55
  %0 = load i32, ptr %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100 x [100 x i32]], ptr @B, i64 0, i64 %indvars.iv58, i64 %indvars.iv55
  store i32 %0, ptr %arrayidx9, align 4
  br label %for.body12

for.body12:                                       ; preds = %for.body3, %for.body12
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body12 ]
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], ptr @C, i64 0, i64 %indvars.iv55, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx16, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx16, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 99
  br i1 %exitcond.not, label %for.end, label %for.body12

for.end:                                          ; preds = %for.body12
  %arrayidx21 = getelementptr inbounds [100 x [100 x i32]], ptr @D, i64 0, i64 %indvars.iv58, i64 %indvars.iv55
  %2 = load i32, ptr %arrayidx21, align 4
  %arrayidx25 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv.next59, i64 %indvars.iv55
  store i32 %2, ptr %arrayidx25, align 4
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next56, 99
  br i1 %exitcond57.not, label %for.inc29, label %for.body3

for.inc29:                                        ; preds = %for.end
  %exitcond60.not = icmp eq i64 %indvars.iv.next59, 99
  br i1 %exitcond60.not, label %for.end31, label %for.cond1.preheader

for.end31:                                        ; preds = %for.inc29
  ret void
}

