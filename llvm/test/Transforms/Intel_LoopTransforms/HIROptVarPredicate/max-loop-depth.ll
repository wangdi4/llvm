; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-var-predicate,print<hir>" -disable-output %s 2>&1 | FileCheck %s

; Test check that OptVarPredicate can update refs on loop of depth 8.

;Function: foo
;<0>          BEGIN REGION { }
;<92>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<93>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<94>               |   |   + DO i3 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<95>               |   |   |   + DO i4 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<96>               |   |   |   |   + DO i5 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<97>               |   |   |   |   |   + DO i6 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<98>               |   |   |   |   |   |   + DO i7 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<21>               |   |   |   |   |   |   |   if (i7 != 0)
;<21>               |   |   |   |   |   |   |   {
;<28>               |   |   |   |   |   |   |      (@b)[0][i1][i2][i3][i4][i5][i6][i7] = (@a)[0][i1][i2][i3][i4][i5][i6][i7][0];
;<21>               |   |   |   |   |   |   |   }
;<99>               |   |   |   |   |   |   |
;<99>               |   |   |   |   |   |   |   + DO i8 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 20>  <LEGAL_MAX_TC = 2147483647>
;<36>               |   |   |   |   |   |   |   |   (@a)[0][i1][i2][i3][i4][i5][i6][i7][i8] = i8;
;<99>               |   |   |   |   |   |   |   + END LOOP
;<98>               |   |   |   |   |   |   + END LOOP
;<97>               |   |   |   |   |   + END LOOP
;<96>               |   |   |   |   + END LOOP
;<95>               |   |   |   + END LOOP
;<94>               |   |   + END LOOP
;<93>               |   + END LOOP
;<92>               + END LOOP
;<0>          END REGION


; Check that OptVarPred pass removed 'if' statement in the i7 loop:
; CHECK: DO
; CHECK-NOT: if (i7 != 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [20 x [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  %cmp171 = icmp sgt i32 %n, 0
  br i1 %cmp171, label %for.cond1.preheader.lr.ph, label %for.end86

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count199 = zext i32 %n to i64
  br label %for.cond4.preheader.lr.ph

for.cond4.preheader.lr.ph:                        ; preds = %for.inc84, %for.cond1.preheader.lr.ph
  %indvars.iv197 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next198, %for.inc84 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc81
  %indvars.iv193 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next194, %for.inc81 ]
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.cond4.preheader, %for.inc78
  %indvars.iv189 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next190, %for.inc78 ]
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond7.preheader, %for.inc75
  %indvars.iv185 = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next186, %for.inc75 ]
  br label %for.cond13.preheader

for.cond13.preheader:                             ; preds = %for.cond10.preheader, %for.inc72
  %indvars.iv181 = phi i64 [ 0, %for.cond10.preheader ], [ %indvars.iv.next182, %for.inc72 ]
  br label %for.cond16.preheader

for.cond16.preheader:                             ; preds = %for.cond13.preheader, %for.inc69
  %indvars.iv177 = phi i64 [ 0, %for.cond13.preheader ], [ %indvars.iv.next178, %for.inc69 ]
  br label %for.body18

for.body18:                                       ; preds = %for.cond16.preheader, %for.inc66
  %indvars.iv173 = phi i64 [ 0, %for.cond16.preheader ], [ %indvars.iv.next174, %for.inc66 ]
  %cmp19.not = icmp eq i64 %indvars.iv173, 0
  br i1 %cmp19.not, label %for.body49.lr.ph, label %if.then

if.then:                                          ; preds = %for.body18
  %arrayidx32 = getelementptr inbounds [20 x [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]]], ptr @a, i64 0, i64 %indvars.iv197, i64 %indvars.iv193, i64 %indvars.iv189, i64 %indvars.iv185, i64 %indvars.iv181, i64 %indvars.iv177, i64 %indvars.iv173, i64 0
  %0 = load i32, ptr %arrayidx32, align 16
  %arrayidx46 = getelementptr inbounds [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]], ptr @b, i64 0, i64 %indvars.iv197, i64 %indvars.iv193, i64 %indvars.iv189, i64 %indvars.iv185, i64 %indvars.iv181, i64 %indvars.iv177, i64 %indvars.iv173
  store i32 %0, ptr %arrayidx46, align 4
  br label %for.body49.lr.ph

for.body49.lr.ph:                                 ; preds = %for.body18, %if.then
  br label %for.body49

for.body49:                                       ; preds = %for.body49.lr.ph, %for.body49
  %indvars.iv = phi i64 [ 0, %for.body49.lr.ph ], [ %indvars.iv.next, %for.body49 ]
  %arrayidx65 = getelementptr inbounds [20 x [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]]], ptr @a, i64 0, i64 %indvars.iv197, i64 %indvars.iv193, i64 %indvars.iv189, i64 %indvars.iv185, i64 %indvars.iv181, i64 %indvars.iv177, i64 %indvars.iv173, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %arrayidx65, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count199
  br i1 %exitcond.not, label %for.inc66, label %for.body49

for.inc66:                                        ; preds = %for.body49
  %indvars.iv.next174 = add nuw nsw i64 %indvars.iv173, 1
  %exitcond176.not = icmp eq i64 %indvars.iv.next174, %wide.trip.count199
  br i1 %exitcond176.not, label %for.inc69, label %for.body18

for.inc69:                                        ; preds = %for.inc66
  %indvars.iv.next178 = add nuw nsw i64 %indvars.iv177, 1
  %exitcond180.not = icmp eq i64 %indvars.iv.next178, %wide.trip.count199
  br i1 %exitcond180.not, label %for.inc72, label %for.cond16.preheader

for.inc72:                                        ; preds = %for.inc69
  %indvars.iv.next182 = add nuw nsw i64 %indvars.iv181, 1
  %exitcond184.not = icmp eq i64 %indvars.iv.next182, %wide.trip.count199
  br i1 %exitcond184.not, label %for.inc75, label %for.cond13.preheader

for.inc75:                                        ; preds = %for.inc72
  %indvars.iv.next186 = add nuw nsw i64 %indvars.iv185, 1
  %exitcond188.not = icmp eq i64 %indvars.iv.next186, %wide.trip.count199
  br i1 %exitcond188.not, label %for.inc78, label %for.cond10.preheader

for.inc78:                                        ; preds = %for.inc75
  %indvars.iv.next190 = add nuw nsw i64 %indvars.iv189, 1
  %exitcond192.not = icmp eq i64 %indvars.iv.next190, %wide.trip.count199
  br i1 %exitcond192.not, label %for.inc81, label %for.cond7.preheader

for.inc81:                                        ; preds = %for.inc78
  %indvars.iv.next194 = add nuw nsw i64 %indvars.iv193, 1
  %exitcond196.not = icmp eq i64 %indvars.iv.next194, %wide.trip.count199
  br i1 %exitcond196.not, label %for.inc84, label %for.cond4.preheader

for.inc84:                                        ; preds = %for.inc81
  %indvars.iv.next198 = add nuw nsw i64 %indvars.iv197, 1
  %exitcond200.not = icmp eq i64 %indvars.iv.next198, %wide.trip.count199
  br i1 %exitcond200.not, label %for.end86.loopexit, label %for.cond4.preheader.lr.ph

for.end86.loopexit:                               ; preds = %for.inc84
  br label %for.end86

for.end86:                                        ; preds = %for.end86.loopexit, %entry
  %div = sdiv i32 %n, 2
  %idxprom87 = sext i32 %div to i64
  %arrayidx109 = getelementptr inbounds [20 x [20 x [20 x [20 x [20 x [20 x [20 x [20 x i32]]]]]]]], ptr @a, i64 0, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87, i64 %idxprom87
  %2 = load i32, ptr %arrayidx109, align 4
  ret i32 %2
}
