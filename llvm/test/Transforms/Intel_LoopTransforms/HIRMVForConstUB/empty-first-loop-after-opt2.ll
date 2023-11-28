; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we still have multiversioning even if the first loop was removed after constant propogation.

;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
;                  |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   if (%n <u 3)
;                  |   |   {
;                  |   |      (@a)[0][i1][i2] = i1;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;
;
;                  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
;                  |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   if (%n >u 3)
;                  |   |   {
;                  |   |      (@b)[0][i1][i2] = i2;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; HIR afetr optimization:

; CHECK:     BEGIN REGION { }
; CHECK:           if (%n == 10)
; CHECK:           {
; CHECK:              + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, 9, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   (@b)[0][i1][i2] = i2;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           else
; CHECK:           {
; CHECK:              + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   if (%n <u 3)
; CHECK:              |   |   {
; CHECK:              |   |      (@a)[0][i1][i2] = i1;
; CHECK:              |   |   }
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP

; CHECK:              + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   if (%n >u 3)
; CHECK:              |   |   {
; CHECK:              |   |      (@b)[0][i1][i2] = i2;
; CHECK:              |   |   }
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:     END REGION



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp63 = icmp sgt i32 %n, 0
  br i1 %cmp63, label %for.cond1.preheader.lr.ph, label %for.end28

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp4 = icmp ult i32 %n, 3
  %wide.trip.count71 = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc7, %for.cond1.preheader.lr.ph
  %indvars.iv69 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next70, %for.inc7 ]
  %0 = trunc i64 %indvars.iv69 to i32
  br label %for.body3

for.cond10.preheader:                             ; preds = %for.inc7
  br label %for.cond13.preheader.lr.ph

for.cond13.preheader.lr.ph:                       ; preds = %for.cond10.preheader
  %cmp16 = icmp ugt i32 %n, 3
  br label %for.body15.lr.ph

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [10 x [10 x i32]], ptr @a, i64 0, i64 %indvars.iv69, i64 %indvars.iv
  store i32 %0, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count71
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.inc
  %indvars.iv.next70 = add nuw nsw i64 %indvars.iv69, 1
  %exitcond72.not = icmp eq i64 %indvars.iv.next70, %wide.trip.count71
  br i1 %exitcond72.not, label %for.cond10.preheader, label %for.body3.lr.ph

for.body15.lr.ph:                                 ; preds = %for.inc26, %for.cond13.preheader.lr.ph
  %indvars.iv77 = phi i64 [ 0, %for.cond13.preheader.lr.ph ], [ %indvars.iv.next78, %for.inc26 ]
  br label %for.body15

for.body15:                                       ; preds = %for.body15.lr.ph, %for.inc23
  %indvars.iv73 = phi i64 [ 0, %for.body15.lr.ph ], [ %indvars.iv.next74, %for.inc23 ]
  br i1 %cmp16, label %if.then17, label %for.inc23

if.then17:                                        ; preds = %for.body15
  %arrayidx21 = getelementptr inbounds [10 x [10 x i32]], ptr @b, i64 0, i64 %indvars.iv77, i64 %indvars.iv73
  %1 = trunc i64 %indvars.iv73 to i32
  store i32 %1, ptr %arrayidx21, align 4 
  br label %for.inc23

for.inc23:                                        ; preds = %for.body15, %if.then17
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next74, %wide.trip.count71
  br i1 %exitcond76.not, label %for.inc26, label %for.body15

for.inc26:                                        ; preds = %for.inc23
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1
  %exitcond80.not = icmp eq i64 %indvars.iv.next78, %wide.trip.count71
  br i1 %exitcond80.not, label %for.end28.loopexit, label %for.body15.lr.ph

for.end28.loopexit:                               ; preds = %for.inc26
  br label %for.end28

for.end28:                                        ; preds = %for.end28.loopexit, %entry
  ret i32 0 
}

