; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we discard optimized code if after MV for const UB 'then'
; branch of the MV condition is empty.

; HIR before optimization:
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 2147483647>
;                  |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 2147483647>
;                  |   |   if (%n <u 3)
;                  |   |   {
;                  |   |      (@a)[0][i1][i2] = i2;
;                  |   |   }
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; No optimization expected
; CHECK:     BEGIN REGION { }
; CHECK-NEXT:      + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 2147483647>
; CHECK:           |   |   if (%n <u 3)
; CHECK:           |   |   {
; CHECK:           |   |      (@a)[0][i1][i2] = i2;
; CHECK:           |   |   }
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [7 x [7 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp26 = icmp sgt i32 %n, 0
  br i1 %cmp26, label %for.cond1.preheader.lr.ph, label %for.end9

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp4 = icmp ult i32 %n, 3
  %wide.trip.count30 = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc7, %for.cond1.preheader.lr.ph
  %indvars.iv28 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next29, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [7 x [7 x i32]], ptr @a, i64 0, i64 %indvars.iv28, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count30
  br i1 %exitcond.not, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.inc
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond31.not = icmp eq i64 %indvars.iv.next29, %wide.trip.count30
  br i1 %exitcond31.not, label %for.end9.loopexit, label %for.body3.lr.ph

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret i32 0 
}
