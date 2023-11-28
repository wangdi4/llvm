; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -S -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Test checks that compiler does not assert about temp (%m) that should be the
; live-in for i3 loop inside second i2 loop in the HIR after optimization.
; Note: loop <79> only have i2 as a part of upper bound.

; HIR before optimization:
;<0>          BEGIN REGION { }
;<77>               + DO i1 = 0, 19, 1   <DO_LOOP>
;<78>               |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<10>               |   |   if (%y > %n)
;<10>               |   |   {
;<79>               |   |      + DO i3 = 0, i2 + -3, 1   <DO_LOOP>  <MAX_TC_EST = 20>
;<24>               |   |      |   %4 = (@c)[0][i3];
;<26>               |   |      |   (@c)[0][i3] = i1 + %4;
;<79>               |   |      + END LOOP
;<10>               |   |   }
;<37>               |   |   if (i2 <= %m)
;<37>               |   |   {
;<80>               |   |      + DO i3 = 0, -1 * i2 + sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>
;<52>               |   |      |   (@b)[0][i1][i2][i3] = i3;
;<80>               |   |      + END LOOP
;<37>               |   |   }
;<78>               |   + END LOOP
;<77>               + END LOOP
;<0>          END REGION

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i64 i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   if (%n > 0)
; CHECK:           |   {
; CHECK:           |      + DO i64 i2 = 0, smin((-1 + sext.i32.i64((1 + %m))), (-1 + sext.i32.i64(%n))), 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |      |   if (%y > %n)
; CHECK:           |      |   {
; CHECK:           |      |      + DO i64 i3 = 0, i2 + -3, 1   <DO_LOOP>  <MAX_TC_EST = 20>
; CHECK:           |      |      |   %4 = (@c)[0][i3];
; CHECK:           |      |      |   (@c)[0][i3] = i1 + %4;
; CHECK:           |      |      + END LOOP
; CHECK:           |      |   }
; CHECK:           |      |
; CHECK:           |      |   + DO i64 i3 = 0, -1 * i2 + sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>
; CHECK:           |      |   |   (@b)[0][i1][i2][i3] = i3;
; CHECK:           |      |   + END LOOP
; CHECK:           |      + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |      + DO i64 i2 = 0, sext.i32.i64(%n) + -1 * smax(0, sext.i32.i64((1 + %m))) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |      |   if (%y > %n)
; CHECK:           |      |   {
; CHECK:           |      |      + LiveIn symbases:{{.*}}, [[SB:.*]]
; CHECK:           |      |      + DO i64 i3 = 0, i2 + smax(0, sext.i32.i64((1 + %m))) + -3, 1   <DO_LOOP>  <MAX_TC_EST = 20>
; CHECK:           |      |      | <RVAL-REG> LINEAR i64 i2 + smax(0, sext.i32.i64((1 + %m))) + -3
; CHECK:           |      |      |    <BLOB> LINEAR i32 %m  {sb:[[SB]]}
; CHECK:           |      |      |   %4 = (@c)[0][i3];
; CHECK:           |      |      |   (@c)[0][i3] = i1 + %4;
; CHECK:           |      |      + END LOOP
; CHECK:           |      |   }
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [20 x [20 x [20 x i32]]] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [20 x [20 x [20 x i32]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n, i32 noundef %m, i32 noundef %y) local_unnamed_addr {
entry:
  %cmp269 = icmp sgt i32 %n, 0
  %cmp4 = icmp sgt i32 %y, %n
  %0 = sext i32 %n to i64
  %1 = sext i32 %m to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc27
  %indvars.iv83 = phi i64 [ 0, %entry ], [ %indvars.iv.next84, %for.inc27 ]
  br i1 %cmp269, label %for.body3.lr.ph, label %for.inc27

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %2 = trunc i64 %indvars.iv83 to i32
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc24
  %indvars.iv77 = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next78, %for.inc24 ]
  br i1 %cmp4, label %for.cond5.preheader, label %if.end

for.cond5.preheader:                              ; preds = %for.body3
  %3 = add nsw i64 %indvars.iv77, -2
  %cmp665 = icmp ugt i64 %indvars.iv77, 2
  br i1 %cmp665, label %for.body7.preheader, label %if.end

for.body7.preheader:                              ; preds = %for.cond5.preheader
  br label %for.body7

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body7 ], [ 0, %for.body7.preheader ]
  %arrayidx = getelementptr inbounds [20 x i32], ptr @c, i64 0, i64 %indvars.iv
  %4 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %4, %2
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %3
  br i1 %exitcond.not, label %if.end.loopexit, label %for.body7

if.end.loopexit:                                  ; preds = %for.body7
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %for.cond5.preheader, %for.body3
  %cmp8.not = icmp sgt i64 %indvars.iv77, %1
  br i1 %cmp8.not, label %for.inc24, label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %if.end
  %5 = sub nsw i64 %0, %indvars.iv77
  %cmp1267 = icmp sgt i64 %5, 0
  br i1 %cmp1267, label %for.body13.preheader, label %for.inc24

for.body13.preheader:                             ; preds = %for.cond10.preheader
  br label %for.body13

for.body13:                                       ; preds = %for.body13.preheader, %for.body13
  %indvars.iv73 = phi i64 [ %indvars.iv.next74, %for.body13 ], [ 0, %for.body13.preheader ]
  %arrayidx19 = getelementptr inbounds [20 x [20 x [20 x i32]]], ptr @b, i64 0, i64 %indvars.iv83, i64 %indvars.iv77, i64 %indvars.iv73
  %6 = trunc i64 %indvars.iv73 to i32
  store i32 %6, ptr %arrayidx19, align 4
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next74, %5
  br i1 %exitcond76.not, label %for.inc24.loopexit, label %for.body13

for.inc24.loopexit:                               ; preds = %for.body13
  br label %for.inc24

for.inc24:                                        ; preds = %for.inc24.loopexit, %for.cond10.preheader, %if.end
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %exitcond82.not = icmp eq i64 %indvars.iv.next78, %0
  br i1 %exitcond82.not, label %for.inc27.loopexit, label %for.body3

for.inc27.loopexit:                               ; preds = %for.inc24
  br label %for.inc27

for.inc27:                                        ; preds = %for.inc27.loopexit, %for.cond1.preheader
  %indvars.iv.next84 = add nuw nsw i64 %indvars.iv83, 1
  %exitcond85.not = icmp eq i64 %indvars.iv.next84, 20
  br i1 %exitcond85.not, label %for.end29, label %for.cond1.preheader

for.end29:                                        ; preds = %for.inc27
  %arrayidx35 = getelementptr inbounds [20 x [20 x [20 x i32]]], ptr @a, i64 0, i64 %0, i64 %0, i64 %0
  %7 = load i32, ptr %arrayidx35, align 4
  %arrayidx41 = getelementptr inbounds [20 x [20 x [20 x i32]]], ptr @b, i64 0, i64 %0, i64 %0, i64 %0
  %8 = load i32, ptr %arrayidx41, align 4
  %add42 = add nsw i32 %8, %7
  ret i32 %add42
}

