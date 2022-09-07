; RUN: opt -passes="hir-ssa-deconstruction,hir-unroll-and-jam,print<hir>" -hir-unroll-and-jam-max-factor=2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s --check-prefix=CHECK-DD

; Verify that i1 loop can be unroll & jammed in the presence of this edge which is
; loop independant at the i1 level-

; CHECK-DD: (@A)[0][i1][i2 + i3 + 1] --> (@A)[0][i1][i2] ANTI (= <)

; Input HIR-
; + DO i1 = 96, 99, 1   <DO_LOOP> <nounroll and jam>
; |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
; |   |   %0 = (@A)[0][i1][i2];
; |   |   %t.0.lcssa = %0;
; |   |
; |   |      %t.049 = %0;
; |   |   + DO i3 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
; |   |   |   %3 = (@A)[0][i1][i2 + i3 + 1];
; |   |   |   %4 = (@B)[0][i2][i3];
; |   |   |   %t.049 = %t.049  +  %3 + %4; <Safe Reduction>
; |   |   + END LOOP
; |   |      %t.0.lcssa = %t.049;
; |   |
; |   |   (@A)[0][i1][i2] = %t.0.lcssa;
; |   + END LOOP
; + END LOOP

; CHECK: modified

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP> <nounroll and jam>
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   %temp = (@A)[0][2 * i1][i2];
; CHECK: |   |   %temp4 = %temp;
; CHECK: |   |   %0 = (@A)[0][2 * i1 + 1][i2];
; CHECK: |   |   %t.0.lcssa = %0;
; CHECK: |   |
; CHECK: |   |      %temp5 = %temp;
; CHECK: |   |      %t.049 = %0;
; CHECK: |   |   + DO i3 = 0, zext.i32.i64(%m) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 99>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   |   %3 = (@A)[0][2 * i1][i2 + i3 + 1];
; CHECK: |   |   |   %4 = (@B)[0][i2][i3];
; CHECK: |   |   |   %temp5 = %temp5  +  %3 + %4;
; CHECK: |   |   |   %3 = (@A)[0][2 * i1 + 1][i2 + i3 + 1];
; CHECK: |   |   |   %4 = (@B)[0][i2][i3];
; CHECK: |   |   |   %t.049 = %t.049  +  %3 + %4;
; CHECK: |   |   + END LOOP
; CHECK: |   |      %temp4 = %temp5;
; CHECK: |   |      %t.0.lcssa = %t.049;
; CHECK: |   |
; CHECK: |   |   (@A)[0][2 * i1][i2] = %temp4;
; CHECK: |   |   (@A)[0][2 * i1 + 1][i2] = %t.0.lcssa;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

define dso_local void @foo(i32 noundef %n, i32 noundef %m) {
entry:
  %cmp250 = icmp sgt i32 %n, 0
  %cmp747 = icmp sgt i32 %m, 0
  %wide.trip.count57 = zext i32 %n to i64
  %wide.trip.count = zext i32 %m to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc27
  %indvars.iv59 = phi i64 [ 0, %entry ], [ %indvars.iv.next60, %for.inc27 ]
  br i1 %cmp250, label %for.body3.preheader, label %for.inc27

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.end
  %indvars.iv55 = phi i64 [ %indvars.iv.next56, %for.end ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv59, i64 %indvars.iv55
  %0 = load i32, i32* %arrayidx5, align 4
  br i1 %cmp747, label %for.body8.preheader, label %for.end

for.body8.preheader:                              ; preds = %for.body3
  br label %for.body8

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body8 ], [ 0, %for.body8.preheader ]
  %t.049 = phi i32 [ %add19, %for.body8 ], [ %0, %for.body8.preheader ]
  %1 = add nuw nsw i64 %indvars.iv55, %indvars.iv
  %2 = add nuw nsw i64 %1, 1
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv59, i64 %2
  %3 = load i32, i32* %arrayidx13, align 4
  %arrayidx17 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %indvars.iv55, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx17, align 4
  %add18 = add nsw i32 %3, %4
  %add19 = add nsw i32 %t.049, %add18
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body8

for.end.loopexit:                                 ; preds = %for.body8
  %add19.lcssa = phi i32 [ %add19, %for.body8 ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body3
  %t.0.lcssa = phi i32 [ %0, %for.body3 ], [ %add19.lcssa, %for.end.loopexit ]
  store i32 %t.0.lcssa, i32* %arrayidx5, align 4
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond58.not = icmp eq i64 %indvars.iv.next56, %wide.trip.count57
  br i1 %exitcond58.not, label %for.inc27.loopexit, label %for.body3

for.inc27.loopexit:                               ; preds = %for.end
  br label %for.inc27

for.inc27:                                        ; preds = %for.inc27.loopexit, %for.cond1.preheader
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next60, 100
  br i1 %exitcond61.not, label %for.end29, label %for.cond1.preheader

for.end29:                                        ; preds = %for.inc27
  ret void
}

