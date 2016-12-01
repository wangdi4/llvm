; RUN: opt < %s -hir-ssa-deconstruction -hir-unroll-and-jam -print-before=hir-unroll-and-jam -print-after=hir-unroll-and-jam 2>&1 | FileCheck %s

; Verify that we unroll i1 loop by 8.

; CHECK: Dump Before HIR Unroll & Jam 

; CHECK: + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %1 = (@A)[0][i1 + 1][i2];
; CHECK: |   |   %2 = (@A)[0][i1][i2];
; CHECK: |   |   (@A)[0][i1][i2] = %1 + %2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: Dump After HIR Unroll & Jam 

; CHECK: BEGIN REGION { modified }
; CHECK: %tgu = (zext.i32.i64((-1 + %n)) + 1)/u8;

; CHECK: + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>  <MAX_TC_EST = 12>
; CHECK: |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 1][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1][i2];
; CHECK: |   |   (@A)[0][8 * i1][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 2][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 1][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 1][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 3][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 2][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 2][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 4][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 3][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 3][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 5][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 4][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 4][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 6][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 5][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 5][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 7][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 6][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 6][i2] = %1 + %2;
; CHECK: |   |   %1 = (@A)[0][8 * i1 + 8][i2];
; CHECK: |   |   %2 = (@A)[0][8 * i1 + 7][i2];
; CHECK: |   |   (@A)[0][8 * i1 + 7][i2] = %1 + %2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: + DO i1 = 8 * %tgu, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 7>
; CHECK: |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %1 = (@A)[0][i1 + 1][i2];
; CHECK: |   |   %2 = (@A)[0][i1][i2];
; CHECK: |   |   (@A)[0][i1][i2] = %1 + %2;
; CHECK: |   + END LOOP
; CHECK: + END LOOP
; CHECK: END REGION


source_filename = "unroll-and-jam-reuse.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %n) {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.body3.lr.ph.preheader, label %for.end13

for.body3.lr.ph.preheader:                        ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc11
  %indvars.iv27 = phi i64 [ %0, %for.inc11 ], [ 0, %for.body3.lr.ph.preheader ]
  %0 = add nuw nsw i64 %indvars.iv27, 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %arrayidx9 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv27, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx9, align 4
  %add10 = add nsw i32 %2, %1
  store i32 %add10, i32* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc11, label %for.body3

for.inc11:                                        ; preds = %for.body3
  %lftr.wideiv30 = trunc i64 %0 to i32
  %exitcond31 = icmp eq i32 %lftr.wideiv30, %n
  br i1 %exitcond31, label %for.end13.loopexit, label %for.body3.lr.ph

for.end13.loopexit:                               ; preds = %for.inc11
  br label %for.end13

for.end13:                                        ; preds = %for.end13.loopexit, %entry
  ret void
}

