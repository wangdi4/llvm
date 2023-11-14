; REQUIRED: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -debug-only=hir-loop-fusion -disable-output %s 2>&1 | FileCheck %s

; Test checks that we don't fuse loops due to difference in UBs.
; DD should not refine the edge between <7> and <30> as independent due to distance between
; i2 and i2 + zext.i32.i64(%n) which is greater than loop <49> UB, because loop <47> has different UB.

;<0>          BEGIN REGION { }
;<46>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<47>               |   + DO i2 = 0, 2 * zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 4294967294>
;<7>                |   |   (@a)[0][i2][i1] = i2;
;<47>               |   + END LOOP
;<46>               + END LOOP
;<46>
;<48>
;<48>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<49>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;<30>               |   |   %2 = (@a)[0][i2 + zext.i32.i64(%n)][i1];
;<32>               |   |   (@b)[0][i2][i1] = %2;
;<49>               |   + END LOOP
;<48>               + END LOOP
;<0>          END REGION

; CHECK:      (@a)[0][i2][i1] --> (@a)[0][i2 + zext.i32.i64(%n)][i1] FLOW (*) (?)
; CHECK:      Forward dep: (*) Backward dep: (* *)  (? ?) < refined >
; CHECK-NEXT: < illegal >

; No Fusion expected:
; CHECK: DO i1
; CHECK: DO i2
; CHECK: DO i1
; CHECK: DO i2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %for.cond.preheader, label %if.end

for.cond.preheader:                               ; preds = %entry
  %mul = shl nuw nsw i32 %n, 1
  %wide.trip.count56 = zext i32 %n to i64
  %wide.trip.count = zext i32 %mul to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond.preheader, %for.inc7
  %indvars.iv54 = phi i64 [ 0, %for.cond.preheader ], [ %indvars.iv.next55, %for.inc7 ]
  br label %for.body4

for.body4:                                        ; preds = %for.cond2.preheader, %for.body4
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx6 = getelementptr inbounds [100 x [100 x i32]], ptr @a, i64 0, i64 %indvars.iv, i64 %indvars.iv54
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc7, label %for.body4

for.inc7:                                         ; preds = %for.body4
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next55, %wide.trip.count56
  br i1 %exitcond57.not, label %for.body15.lr.ph.preheader, label %for.cond2.preheader

for.body15.lr.ph.preheader:                       ; preds = %for.inc7
  br label %for.body15.lr.ph

for.body15.lr.ph:                                 ; preds = %for.body15.lr.ph.preheader, %for.inc27
  %indvars.iv63 = phi i64 [ %indvars.iv.next64, %for.inc27 ], [ 0, %for.body15.lr.ph.preheader ]
  br label %for.body15

for.body15:                                       ; preds = %for.body15.lr.ph, %for.body15
  %indvars.iv58 = phi i64 [ 0, %for.body15.lr.ph ], [ %indvars.iv.next59, %for.body15 ]
  %1 = add nuw nsw i64 %indvars.iv58, %wide.trip.count56
  %arrayidx19 = getelementptr inbounds [100 x [100 x i32]], ptr @a, i64 0, i64 %1, i64 %indvars.iv63
  %2 = load i32, ptr %arrayidx19, align 4
  %arrayidx23 = getelementptr inbounds [100 x [100 x i32]], ptr @b, i64 0, i64 %indvars.iv58, i64 %indvars.iv63
  store i32 %2, ptr %arrayidx23, align 4
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond62.not = icmp eq i64 %indvars.iv.next59, %wide.trip.count56
  br i1 %exitcond62.not, label %for.inc27, label %for.body15

for.inc27:                                        ; preds = %for.body15
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond66.not = icmp eq i64 %indvars.iv.next64, %wide.trip.count56
  br i1 %exitcond66.not, label %if.end.loopexit, label %for.body15.lr.ph

if.end.loopexit:                                  ; preds = %for.inc27
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %entry
  ret i32 undef
}

