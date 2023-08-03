; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that MV for const TC doesn't happen because it will nullify the i1 loop.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, sext.i32.i64(%N) + -7, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   if (%N > 0)
;                  |   {
;                  |      + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |      |   %0 = (@A)[0][i1][i2];
;                  |      |   (@A)[0][i1][i2] = %0 + 2;
;                  |      + END LOOP
;                  |
;                  |
;                  |      + DO i2 = 0, zext.i32.i64((-3 + %N)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483644>
;                  |      |   %1 = (@A)[0][i1][i2];
;                  |      |   (@A)[0][i1][i2] = i2 + %1;
;                  |      + END LOOP
;                  |   }
;                  + END LOOP
;            END REGION


; Check that no multiversioning happened:

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:      + DO i1 = 0, sext.i32.i64(%N) + -7, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NEXT:      |   if (%N > 0)
; CHECK-NEXT:      |   {
; CHECK-NEXT:      |      + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |      |   %0 = (@A)[0][i1][i2];
;                  |      |   (@A)[0][i1][i2] = %0 + 2;
; CHECK:           |      + END LOOP
;                  |
;                  |
; CHECK:           |      + DO i2 = 0, zext.i32.i64((-3 + %N)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483644>
;                  |      |   %1 = (@A)[0][i1][i2];
;                  |      |   (@A)[0][i1][i2] = i2 + %1;
; CHECK:           |      + END LOOP
; CHECK-NEXT:      |   }
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [5 x [5 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %N) local_unnamed_addr {
entry:
  %sub = add nsw i32 %N, -6
  %cmp42 = icmp sgt i32 %sub, 0
  br i1 %cmp42, label %for.cond1.preheader.lr.ph, label %for.end20

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp238 = icmp sgt i32 %N, 0
  %sub7 = add nsw i32 %N, -3
  %cmp840 = icmp sgt i32 %N, 3
  %wide.trip.count50 = zext i32 %sub to i64
  %wide.trip.count = zext i32 %N to i64
  %wide.trip.count46 = zext i32 %sub7 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.inc18
  %indvars.iv48 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next49, %for.inc18 ]
  br i1 %cmp238, label %for.body3.preheader, label %for.inc18

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.cond6.preheader:                              ; preds = %for.body3
  br i1 %cmp840, label %for.body9.preheader, label %for.inc18

for.body9.preheader:                              ; preds = %for.cond6.preheader
  br label %for.body9

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv48, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond6.preheader, label %for.body3

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv44 = phi i64 [ %indvars.iv.next45, %for.body9 ], [ 0, %for.body9.preheader ]
  %arrayidx13 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv48, i64 %indvars.iv44
  %1 = load i32, ptr %arrayidx13, align 4
  %2 = trunc i64 %indvars.iv44 to i32
  %add14 = add nsw i32 %1, %2
  store i32 %add14, ptr %arrayidx13, align 4
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond47.not = icmp eq i64 %indvars.iv.next45, %wide.trip.count46
  br i1 %exitcond47.not, label %for.inc18.loopexit, label %for.body9

for.inc18.loopexit:                               ; preds = %for.body9
  br label %for.inc18

for.inc18:                                        ; preds = %for.inc18.loopexit, %for.cond1.preheader, %for.cond6.preheader
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51.not = icmp eq i64 %indvars.iv.next49, %wide.trip.count50
  br i1 %exitcond51.not, label %for.end20.loopexit, label %for.cond1.preheader

for.end20.loopexit:                               ; preds = %for.inc18
  br label %for.end20

for.end20:                                        ; preds = %for.end20.loopexit, %entry
  %div = sdiv i32 %N, 2
  %idxprom21 = sext i32 %div to i64
  %div23 = sdiv i32 %N, 3
  %idxprom24 = sext i32 %div23 to i64
  %arrayidx25 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %idxprom21, i64 %idxprom24
  %3 = load i32, ptr %arrayidx25, align 4
  ret i32 %3
}

