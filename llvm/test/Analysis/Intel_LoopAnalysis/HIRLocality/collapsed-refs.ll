; RUN: opt -passes='hir-ssa-deconstruction,hir-loop-collapse,hir-post-vec-complete-unroll,print<hir>,print<hir-locality-analysis>' -hir-spatial-locality -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not crash while trying to analyze collapsed ref (@A)[0][0][3]
; and non-collapsed ref (@A)[0][2][0] in the i1 loop. It is not possible for 
; current utilities to compute the correct distance between these two refs as
; the indices for collapsed refs are out of range.

; Original HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 2, 1   <DO_LOOP>
; |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; |   |   |   %0 = (@B)[0][i2][i3];
; |   |   |   %1 = (@A)[0][i2][i3];
; |   |   |   (@A)[0][i2][i3] = %0 + %1;
; |   |   + END LOOP
; |   + END LOOP
; |
; |   (@A)[0][2][0] = 5;
; + END LOOP

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@B)[0][0][0];
; CHECK: |   %1 = (@A)[0][0][0];
; CHECK: |   (@A)[0][0][0] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][1];
; CHECK: |   %1 = (@A)[0][0][1];
; CHECK: |   (@A)[0][0][1] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][2];
; CHECK: |   %1 = (@A)[0][0][2];
; CHECK: |   (@A)[0][0][2] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][3];
; CHECK: |   %1 = (@A)[0][0][3];
; CHECK: |   (@A)[0][0][3] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][4];
; CHECK: |   %1 = (@A)[0][0][4];
; CHECK: |   (@A)[0][0][4] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][5];
; CHECK: |   %1 = (@A)[0][0][5];
; CHECK: |   (@A)[0][0][5] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][6];
; CHECK: |   %1 = (@A)[0][0][6];
; CHECK: |   (@A)[0][0][6] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][7];
; CHECK: |   %1 = (@A)[0][0][7];
; CHECK: |   (@A)[0][0][7] = %0 + %1;
; CHECK: |   %0 = (@B)[0][0][8];
; CHECK: |   %1 = (@A)[0][0][8];
; CHECK: |   (@A)[0][0][8] = %0 + %1;
; CHECK: |   (@A)[0][2][0] = 5;
; CHECK: + END LOOP

; CHECK: NumCacheLines: 3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [3 x [3 x i32]] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [3 x [3 x i32]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.end15
  %i.031 = phi i32 [ 0, %entry ], [ %inc17, %for.end15 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc13
  %indvars.iv32 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next33, %for.inc13 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [3 x [3 x i32]], ptr @B, i64 0, i64 %indvars.iv32, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx8, align 4
  %arrayidx12 = getelementptr inbounds [3 x [3 x i32]], ptr @A, i64 0, i64 %indvars.iv32, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx12, align 4
  %add = add nsw i32 %1, %0
  store i32 %add, ptr %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond.not, label %for.inc13, label %for.body6

for.inc13:                                        ; preds = %for.body6
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34.not = icmp eq i64 %indvars.iv.next33, 3
  br i1 %exitcond34.not, label %for.end15, label %for.cond4.preheader

for.end15:                                        ; preds = %for.inc13
  store i32 5, ptr getelementptr inbounds ([3 x [3 x i32]], ptr @A, i64 0, i64 2, i64 0), align 8
  %inc17 = add nuw nsw i32 %i.031, 1
  %exitcond35.not = icmp eq i32 %inc17, 100
  br i1 %exitcond35.not, label %for.end18, label %for.cond1.preheader

for.end18:                                        ; preds = %for.end15
  ret void
}

