; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we do loopnest multiversioning if innermost loop has small MAX_TC_EST.
; Note that sibling loop became redundant after multiversioning and was removed.

; HIR Before transformation:
;            BEGIN REGION { }
;                  if (%N >u 5)
;                  {
;                     + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   |   %0 = (@A)[0][i1][i2];
;                     |   |   (@A)[0][i1][i2] = %0 + 2;
;                     |   + END LOOP
;                     |
;                     |
;                     |   + DO i2 = 0, sext.i32.i64(%N) + -6, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483642>
;                     |   |   %1 = (@A)[0][i1][i2];
;                     |   |   (@A)[0][i1][i2] = i2 + %1;
;                     |   + END LOOP
;                     + END LOOP
;                  }
;                  else
;                  {
;                     + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   |   %0 = (@A)[0][i1][i2];
;                     |   |   (@A)[0][i1][i2] = %0 + 2;
;                     |   + END LOOP
;                     + END LOOP
;                  }
;            END REGION


; HIR After trasformation:
; CHECK:           if (%N == 5)
; CHECK:           {
; CHECK:              + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %0 = (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = %0 + 2;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
;                     + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                     |   |   %0 = (@A)[0][i1][i2];
;                     |   |   (@A)[0][i1][i2] = %0 + 2;
;                     |   + END LOOP
;                     |
;                     |
;                     |   + DO i2 = 0, sext.i32.i64(%N) + -6, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483642>
;                     |   |   %1 = (@A)[0][i1][i2];
;                     |   |   (@A)[0][i1][i2] = i2 + %1;
;                     |   + END LOOP
;                     + END LOOP
;                  }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [5 x [5 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %N) local_unnamed_addr {
entry:
  %cmp41 = icmp sgt i32 %N, 0
  br i1 %cmp41, label %for.cond1.preheader.lr.ph, label %for.end19

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %sub = add nsw i32 %N, -5
  %cmp739 = icmp ugt i32 %N, 5
  %wide.trip.count49 = zext i32 %N to i64
  %wide.trip.count45 = sext i32 %sub to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc17, %for.cond1.preheader.lr.ph
  %indvars.iv47 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next48, %for.inc17 ]
  br label %for.body3

for.cond6.preheader:                              ; preds = %for.body3
  br i1 %cmp739, label %for.body8.preheader, label %for.inc17

for.body8.preheader:                              ; preds = %for.cond6.preheader
  br label %for.body8

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv47, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count49
  br i1 %exitcond.not, label %for.cond6.preheader, label %for.body3

for.body8:                                        ; preds = %for.body8.preheader, %for.body8
  %indvars.iv43 = phi i64 [ %indvars.iv.next44, %for.body8 ], [ 0, %for.body8.preheader ]
  %arrayidx12 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv47, i64 %indvars.iv43
  %1 = load i32, ptr %arrayidx12, align 4
  %2 = trunc i64 %indvars.iv43 to i32
  %add13 = add nsw i32 %1, %2
  store i32 %add13, ptr %arrayidx12, align 4
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond46.not = icmp eq i64 %indvars.iv.next44, %wide.trip.count45
  br i1 %exitcond46.not, label %for.inc17.loopexit, label %for.body8

for.inc17.loopexit:                               ; preds = %for.body8
  br label %for.inc17

for.inc17:                                        ; preds = %for.inc17.loopexit, %for.cond6.preheader
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond50.not = icmp eq i64 %indvars.iv.next48, %wide.trip.count49
  br i1 %exitcond50.not, label %for.end19.loopexit, label %for.body3.lr.ph

for.end19.loopexit:                               ; preds = %for.inc17
  br label %for.end19

for.end19:                                        ; preds = %for.end19.loopexit, %entry
  %div = sdiv i32 %N, 2
  %idxprom20 = sext i32 %div to i64
  %div22 = sdiv i32 %N, 3
  %idxprom23 = sext i32 %div22 to i64
  %arrayidx24 = getelementptr inbounds [5 x [5 x i32]], ptr @A, i64 0, i64 %idxprom20, i64 %idxprom23
  %3 = load i32, ptr %arrayidx24, align 4
  ret i32 %3
}
