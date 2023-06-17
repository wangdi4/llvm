; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we put first and second loopnests under same multiversioning condition.
; Although last loop nest have the same condition, it is treated separately due to
; interleaving loop nest #3.

; HIR before optimization: 
;            BEGIN REGION { }
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %0 = (@A)[0][i1][i2];
;                  |   |   (@A)[0][i1][i2] = %N + %0;
;                  |   + END LOOP
;                  + END LOOP
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %1 = (@A)[0][i1][i2];
;                  |   |   (@A)[0][i1][i2] = i2 + %1;
;                  |   + END LOOP
;                  + END LOOP
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %3 = (@B)[0][i1][i2];
;                  |   |   (@B)[0][i1][i2] = i2 + %3;
;                  |   + END LOOP
;                  + END LOOP
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %6 = (@A)[0][i1][i2];
;                  |   |   (@A)[0][i1][i2] = i1 + %6;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR after optimization:
; CHECK:           if (%N == 5)
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:           }
; CHECK:           if (%N == 6)
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 5, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:           if (%N == 5)
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + END LOOP
;                     + END LOOP
;                  }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [5 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [6 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %N, i32 noundef %M) local_unnamed_addr {
entry:
  %cmp295 = icmp sgt i32 %N, 0
  %wide.trip.count = zext i32 %N to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc6
  %indvars.iv107 = phi i64 [ 0, %entry ], [ %indvars.iv.next108, %for.inc6 ]
  br i1 %cmp295, label %for.body3.preheader, label %for.inc6

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.cond9.preheader:                              ; preds = %for.inc6
  br label %for.cond12.preheader

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv107, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, %N
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc6.loopexit, label %for.body3

for.inc6.loopexit:                                ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %indvars.iv.next108 = add nuw nsw i64 %indvars.iv107, 1
  %exitcond109.not = icmp eq i64 %indvars.iv.next108, 20
  br i1 %exitcond109.not, label %for.cond9.preheader, label %for.cond1.preheader

for.cond12.preheader:                             ; preds = %for.cond9.preheader, %for.inc23
  %indvars.iv114 = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next115, %for.inc23 ]
  br i1 %cmp295, label %for.body14.preheader, label %for.inc23

for.body14.preheader:                             ; preds = %for.cond12.preheader
  br label %for.body14

for.cond26.preheader:                             ; preds = %for.inc23
  br label %for.cond29.preheader

for.body14:                                       ; preds = %for.body14.preheader, %for.body14
  %indvars.iv110 = phi i64 [ %indvars.iv.next111, %for.body14 ], [ 0, %for.body14.preheader ]
  %arrayidx18 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv114, i64 %indvars.iv110
  %1 = load i32, ptr %arrayidx18, align 4
  %2 = trunc i64 %indvars.iv110 to i32
  %add19 = add nsw i32 %1, %2
  store i32 %add19, ptr %arrayidx18, align 4
  %indvars.iv.next111 = add nuw nsw i64 %indvars.iv110, 1
  %exitcond113.not = icmp eq i64 %indvars.iv.next111, %wide.trip.count
  br i1 %exitcond113.not, label %for.inc23.loopexit, label %for.body14

for.inc23.loopexit:                               ; preds = %for.body14
  br label %for.inc23

for.inc23:                                        ; preds = %for.inc23.loopexit, %for.cond12.preheader
  %indvars.iv.next115 = add nuw nsw i64 %indvars.iv114, 1
  %exitcond116.not = icmp eq i64 %indvars.iv.next115, 20
  br i1 %exitcond116.not, label %for.cond26.preheader, label %for.cond12.preheader

for.cond29.preheader:                             ; preds = %for.cond26.preheader, %for.inc40
  %indvars.iv121 = phi i64 [ 0, %for.cond26.preheader ], [ %indvars.iv.next122, %for.inc40 ]
  br i1 %cmp295, label %for.body31.preheader, label %for.inc40

for.body31.preheader:                             ; preds = %for.cond29.preheader
  br label %for.body31

for.cond43.preheader:                             ; preds = %for.inc40
  br label %for.cond46.preheader

for.body31:                                       ; preds = %for.body31.preheader, %for.body31
  %indvars.iv117 = phi i64 [ %indvars.iv.next118, %for.body31 ], [ 0, %for.body31.preheader ]
  %arrayidx35 = getelementptr inbounds [100 x [6 x i32]], ptr @B, i64 0, i64 %indvars.iv121, i64 %indvars.iv117
  %3 = load i32, ptr %arrayidx35, align 4
  %4 = trunc i64 %indvars.iv117 to i32
  %add36 = add nsw i32 %3, %4
  store i32 %add36, ptr %arrayidx35, align 4
  %indvars.iv.next118 = add nuw nsw i64 %indvars.iv117, 1
  %exitcond120.not = icmp eq i64 %indvars.iv.next118, %wide.trip.count
  br i1 %exitcond120.not, label %for.inc40.loopexit, label %for.body31

for.inc40.loopexit:                               ; preds = %for.body31
  br label %for.inc40

for.inc40:                                        ; preds = %for.inc40.loopexit, %for.cond29.preheader
  %indvars.iv.next122 = add nuw nsw i64 %indvars.iv121, 1
  %exitcond123.not = icmp eq i64 %indvars.iv.next122, 20
  br i1 %exitcond123.not, label %for.cond43.preheader, label %for.cond29.preheader

for.cond46.preheader:                             ; preds = %for.cond43.preheader, %for.inc57
  %indvars.iv128 = phi i64 [ 0, %for.cond43.preheader ], [ %indvars.iv.next129, %for.inc57 ]
  br i1 %cmp295, label %for.body48.lr.ph, label %for.inc57

for.body48.lr.ph:                                 ; preds = %for.cond46.preheader
  %5 = trunc i64 %indvars.iv128 to i32
  br label %for.body48

for.body48:                                       ; preds = %for.body48.lr.ph, %for.body48
  %indvars.iv124 = phi i64 [ 0, %for.body48.lr.ph ], [ %indvars.iv.next125, %for.body48 ]
  %arrayidx52 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv128, i64 %indvars.iv124
  %6 = load i32, ptr %arrayidx52, align 4
  %add53 = add nsw i32 %6, %5
  store i32 %add53, ptr %arrayidx52, align 4
  %indvars.iv.next125 = add nuw nsw i64 %indvars.iv124, 1
  %exitcond127.not = icmp eq i64 %indvars.iv.next125, %wide.trip.count
  br i1 %exitcond127.not, label %for.inc57.loopexit, label %for.body48

for.inc57.loopexit:                               ; preds = %for.body48
  br label %for.inc57

for.inc57:                                        ; preds = %for.inc57.loopexit, %for.cond46.preheader
  %indvars.iv.next129 = add nuw nsw i64 %indvars.iv128, 1
  %exitcond130.not = icmp eq i64 %indvars.iv.next129, 20
  br i1 %exitcond130.not, label %for.end59, label %for.cond46.preheader

for.end59:                                        ; preds = %for.inc57
  %idxprom60 = sext i32 %N to i64
  %arrayidx61 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 50, i64 %idxprom60
  %7 = load i32, ptr %arrayidx61, align 4
  %idxprom62 = sext i32 %M to i64
  %arrayidx63 = getelementptr inbounds [100 x [6 x i32]], ptr @B, i64 0, i64 10, i64 %idxprom62
  %8 = load i32, ptr %arrayidx63, align 4
  %add64 = add nsw i32 %8, %7
  ret i32 %add64
}

