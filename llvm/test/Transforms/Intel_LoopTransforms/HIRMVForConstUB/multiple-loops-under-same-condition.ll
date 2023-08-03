; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we put first and second loopnests under same multiversioning condition.


; HIR before optimization:

;            BEGIN REGION { }
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %0 = (@A)[0][i1][i2];
;                  |   |   (@A)[0][i1][i2] = %N + %0;
;                  |   + END LOOP
;                  + END LOOP
;    
;                  %idxprom9 = sext.i32.i64(%M);
;                  (@A)[0][%M][%M] = %N;
;    
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %1 = (@A)[0][i1][i2];
;                  |   |   (@A)[0][i1][i2] = i2 + %1;
;                  |   + END LOOP
;                  + END LOOP
;    
;                  %idxprom30 = sext.i32.i64(%N);
;                  (@A)[0][%N][%N] = %M;
;    
;                  + DO i1 = 0, 19, 1   <DO_LOOP>
;                  |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   %3 = (@B)[0][i1][i2];
;                  |   |   (@B)[0][i1][i2] = i2 + %3;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION


; HIR after optimization:

; CHECK:     BEGIN REGION { }
; CHECK:           if (%N == 5)
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %0 = (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = %0 + 5;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;        
; CHECK:              %idxprom9 = sext.i32.i64(%M);
; CHECK:              (@A)[0][%M][%M] = 5;
;        
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 4, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %1 = (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = i2 + %1;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %0 = (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = %N + %0;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;     
; CHECK:              %idxprom9 = sext.i32.i64(%M);
; CHECK:              (@A)[0][%M][%M] = %N;
;          
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 5>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %1 = (@A)[0][i1][i2];
; CHECK:              |   |   (@A)[0][i1][i2] = i2 + %1;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           %idxprom30 = sext.i32.i64(%N);
; CHECK:           (@A)[0][%N][%N] = %M;
; CHECK:           if (%N == 6)
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 5, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %3 = (@B)[0][i1][i2];
; CHECK:              |   |   (@B)[0][i1][i2] = i2 + %3;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 6>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   %3 = (@B)[0][i1][i2];
; CHECK:              |   |   (@B)[0][i1][i2] = i2 + %3;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x [5 x i32]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x [6 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %N, i32 noundef %M) local_unnamed_addr {
entry:
  %cmp284 = icmp sgt i32 %N, 0
  %wide.trip.count = zext i32 %N to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc6
  %indvars.iv93 = phi i64 [ 0, %entry ], [ %indvars.iv.next94, %for.inc6 ]
  br i1 %cmp284, label %for.body3.preheader, label %for.inc6

for.body3.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body3

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx5 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv93, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx5, align 4
  %add = add nsw i32 %0, %N
  store i32 %add, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc6.loopexit, label %for.body3

for.inc6.loopexit:                                ; preds = %for.body3
  br label %for.inc6

for.inc6:                                         ; preds = %for.inc6.loopexit, %for.cond1.preheader
  %indvars.iv.next94 = add nuw nsw i64 %indvars.iv93, 1
  %exitcond95.not = icmp eq i64 %indvars.iv.next94, 20
  br i1 %exitcond95.not, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  %idxprom9 = sext i32 %M to i64
  %arrayidx12 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %idxprom9, i64 %idxprom9
  store i32 %N, ptr %arrayidx12, align 4
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %for.end8, %for.inc27
  %indvars.iv100 = phi i64 [ 0, %for.end8 ], [ %indvars.iv.next101, %for.inc27 ]
  br i1 %cmp284, label %for.body18.preheader, label %for.inc27

for.body18.preheader:                             ; preds = %for.cond16.preheader
  br label %for.body18

for.body18:                                       ; preds = %for.body18.preheader, %for.body18
  %indvars.iv96 = phi i64 [ %indvars.iv.next97, %for.body18 ], [ 0, %for.body18.preheader ]
  %arrayidx22 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %indvars.iv100, i64 %indvars.iv96
  %1 = load i32, ptr %arrayidx22, align 4
  %2 = trunc i64 %indvars.iv96 to i32
  %add23 = add nsw i32 %1, %2
  store i32 %add23, ptr %arrayidx22, align 4
  %indvars.iv.next97 = add nuw nsw i64 %indvars.iv96, 1
  %exitcond99.not = icmp eq i64 %indvars.iv.next97, %wide.trip.count
  br i1 %exitcond99.not, label %for.inc27.loopexit, label %for.body18

for.inc27.loopexit:                               ; preds = %for.body18
  br label %for.inc27

for.inc27:                                        ; preds = %for.inc27.loopexit, %for.cond16.preheader
  %indvars.iv.next101 = add nuw nsw i64 %indvars.iv100, 1
  %exitcond102.not = icmp eq i64 %indvars.iv.next101, 20
  br i1 %exitcond102.not, label %for.end29, label %for.cond16.preheader

for.end29:                                        ; preds = %for.inc27
  %idxprom30 = sext i32 %N to i64
  %arrayidx33 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 %idxprom30, i64 %idxprom30
  store i32 %M, ptr %arrayidx33, align 4
  br label %for.cond37.preheader

for.cond37.preheader:                             ; preds = %for.end29, %for.inc48
  %indvars.iv107 = phi i64 [ 0, %for.end29 ], [ %indvars.iv.next108, %for.inc48 ]
  br i1 %cmp284, label %for.body39.preheader, label %for.inc48

for.body39.preheader:                             ; preds = %for.cond37.preheader
  br label %for.body39

for.body39:                                       ; preds = %for.body39.preheader, %for.body39
  %indvars.iv103 = phi i64 [ %indvars.iv.next104, %for.body39 ], [ 0, %for.body39.preheader ]
  %arrayidx43 = getelementptr inbounds [100 x [6 x i32]], ptr @B, i64 0, i64 %indvars.iv107, i64 %indvars.iv103
  %3 = load i32, ptr %arrayidx43, align 4
  %4 = trunc i64 %indvars.iv103 to i32
  %add44 = add nsw i32 %3, %4
  store i32 %add44, ptr %arrayidx43, align 4
  %indvars.iv.next104 = add nuw nsw i64 %indvars.iv103, 1
  %exitcond106.not = icmp eq i64 %indvars.iv.next104, %idxprom30
  br i1 %exitcond106.not, label %for.inc48.loopexit, label %for.body39

for.inc48.loopexit:                               ; preds = %for.body39
  br label %for.inc48

for.inc48:                                        ; preds = %for.inc48.loopexit, %for.cond37.preheader
  %indvars.iv.next108 = add nuw nsw i64 %indvars.iv107, 1
  %exitcond109.not = icmp eq i64 %indvars.iv.next108, 20
  br i1 %exitcond109.not, label %for.end50, label %for.cond37.preheader

for.end50:                                        ; preds = %for.inc48
  %arrayidx52 = getelementptr inbounds [100 x [5 x i32]], ptr @A, i64 0, i64 50, i64 %idxprom30
  %5 = load i32, ptr %arrayidx52, align 4
  %arrayidx54 = getelementptr inbounds [100 x [6 x i32]], ptr @B, i64 0, i64 10, i64 %idxprom9
  %6 = load i32, ptr %arrayidx54, align 4
  %add55 = add nsw i32 %6, %5
  ret i32 %add55
}

