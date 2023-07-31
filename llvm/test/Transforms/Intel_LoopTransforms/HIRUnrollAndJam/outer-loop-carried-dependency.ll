; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -hir-unroll-and-jam-max-factor=2 -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we can unroll & jam i2 loop in the presence of this edge-
; (@A)[0][i1 + 1][i2 + 1][i3] -> (@A)[0][i1][i2][i3] FLOW (< <)

; This dependency is carried by the i1 loop so shouldn't prevent unroll & jam
; of i2 loop.

; Input HIR-
; + DO i1 = 0, 30, 1   <DO_LOOP>
; |   + DO i2 = 0, 29, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; |   |   |   (@B)[0][i1][i2][i3] = (@A)[0][i1][i2][i3];
; |   |   |   %1 = (@C)[0][i1][i3];
; |   |   |   (@C)[0][i1][i3] = %1 + 1;
; |   |   + END LOOP
; |   |
; |   |
; |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; |   |   |   (@A)[0][i1 + 1][i2 + 1][i3] = (@D)[0][i1][i2][i3];
; |   |   + END LOOP
; |   + END LOOP
; + END LOOP

; CHECK: + DO i1 = 0, 30, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 14, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; CHECK: |   |   |   (@B)[0][i1][2 * i2][i3] = (@A)[0][i1][2 * i2][i3];
; CHECK: |   |   |   %1 = (@C)[0][i1][i3];
; CHECK: |   |   |   (@C)[0][i1][i3] = %1 + 1;
; CHECK: |   |   |   (@B)[0][i1][2 * i2 + 1][i3] = (@A)[0][i1][2 * i2 + 1][i3];
; CHECK: |   |   |   %1 = (@C)[0][i1][i3];
; CHECK: |   |   |   (@C)[0][i1][i3] = %1 + 1;
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, 30, 1   <DO_LOOP>
; CHECK: |   |   |   (@A)[0][i1 + 1][2 * i2 + 1][i3] = (@D)[0][i1][2 * i2][i3];
; CHECK: |   |   |   (@A)[0][i1 + 1][2 * i2 + 2][i3] = (@D)[0][i1][2 * i2 + 1][i3];
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [32 x [32 x i32]] zeroinitializer, align 16
@D = dso_local local_unnamed_addr global [32 x [32 x [32 x i32]]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc44
  %indvars.iv79 = phi i64 [ 0, %entry ], [ %indvars.iv.next80, %for.inc44 ]
  %indvars.iv.next80 = add nuw nsw i64 %indvars.iv79, 1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc41
  %indvars.iv76 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next77, %for.inc41 ]
  br label %for.body6

for.cond22.preheader:                             ; preds = %for.body6
  %indvars.iv.next77 = add nuw nsw i64 %indvars.iv76, 1
  br label %for.body24

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @A, i64 0, i64 %indvars.iv79, i64 %indvars.iv76, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx10, align 4
  %arrayidx16 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @B, i64 0, i64 %indvars.iv79, i64 %indvars.iv76, i64 %indvars.iv
  store i32 %0, ptr %arrayidx16, align 4
  %arrayidx20 = getelementptr inbounds [32 x [32 x i32]], ptr @C, i64 0, i64 %indvars.iv79, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx20, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %arrayidx20, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 31
  br i1 %exitcond.not, label %for.cond22.preheader, label %for.body6

for.body24:                                       ; preds = %for.cond22.preheader, %for.body24
  %indvars.iv73 = phi i64 [ 0, %for.cond22.preheader ], [ %indvars.iv.next74, %for.body24 ]
  %arrayidx30 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @D, i64 0, i64 %indvars.iv79, i64 %indvars.iv76, i64 %indvars.iv73
  %2 = load i32, ptr %arrayidx30, align 4
  %arrayidx37 = getelementptr inbounds [32 x [32 x [32 x i32]]], ptr @A, i64 0, i64 %indvars.iv.next80, i64 %indvars.iv.next77, i64 %indvars.iv73
  store i32 %2, ptr %arrayidx37, align 4
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond75.not = icmp eq i64 %indvars.iv.next74, 31
  br i1 %exitcond75.not, label %for.inc41, label %for.body24

for.inc41:                                        ; preds = %for.body24
  %exitcond78.not = icmp eq i64 %indvars.iv.next77, 30
  br i1 %exitcond78.not, label %for.inc44, label %for.cond4.preheader

for.inc44:                                        ; preds = %for.inc41
  %exitcond81.not = icmp eq i64 %indvars.iv.next80, 31
  br i1 %exitcond81.not, label %for.end46, label %for.cond1.preheader

for.end46:                                        ; preds = %for.inc44
  ret void
}

