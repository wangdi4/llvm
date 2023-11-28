; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s --check-prefix=NOOPT
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-mv-const-ub,print<hir>" -disable-hir-mv-const-ub-for-multi-exit-loops=false -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that the multi-exit for the outermost loop is updated
; after the transformation is applied, and the HIR verifier doesn't produce an
; assertion.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;       |   %0 = (@b)[0][i1];
;       |
;       |   + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;       |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
;       |   |   |   %ld0 = (@c)[0][i3];
;       |   |   |   if (%ld0 > 2)
;       |   |   |   {
;       |   |   |      if (%ld0 > 32)
;       |   |   |      {
;       |   |   |         goto for.end.split.loopexit;
;       |   |   |      }
;       |   |   |   }
;       |   |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
;       |   |   + END LOOP
;       |   + END LOOP
;       |      %k.050 = smax(0, %0);
;       + END LOOP
; END REGION


; This optimization is disabled by default:
; NOOPT:  BEGIN REGION { }
; NOOPT-NOT: else


; HIR after transformation when it is enabled:
; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %0 = (@b)[0][i1];
; CHECK:       |   if (%0 == 3)
; CHECK:       |   {
; CHECK:       |      + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:       |      |   + DO i3 = 0, 2, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |      |   |   %ld0 = (@c)[0][i3];
; CHECK:       |      |   |   if (%ld0 > 2)
; CHECK:       |      |   |   {
; CHECK:       |      |   |      if (%ld0 > 32)
; CHECK:       |      |   |      {
; CHECK:       |      |   |         goto for.end.split.loopexit;
; CHECK:       |      |   |      }
; CHECK:       |      |   |   }
; CHECK:       |      |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
; CHECK:       |      |   + END LOOP
; CHECK:       |      + END LOOP
; CHECK:       |         %k.050 = 3;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      + DO i2 = 0, zext.i32.i64(%l) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |      |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |      |   |   %ld0 = (@c)[0][i3];
; CHECK:       |      |   |   if (%ld0 > 2)
; CHECK:       |      |   |   {
; CHECK:       |      |   |      if (%ld0 > 32)
; CHECK:       |      |   |      {
; CHECK:       |      |   |         goto for.end.split.loopexit;
; CHECK:       |      |   |      }
; CHECK:       |      |   |   }
; CHECK:       |      |   |   (@a)[0][i1][i2][i3] = i1 + i2 + i3;
; CHECK:       |      |   + END LOOP
; CHECK:       |      + END LOOP
; CHECK:       |         %k.050 = smax(0, %0);
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x [100 x [3 x i32]]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooiii(i32 noundef %n, i32 noundef %l, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %cmp48 = icmp sgt i32 %n, 0
  br i1 %cmp48, label %for.body.lr.ph, label %for.end19

for.body.lr.ph:                                   ; preds = %entry
  %cmp245 = icmp sgt i32 %l, 0
  %wide.trip.count60 = zext i32 %n to i64
  %wide.trip.count56 = zext i32 %l to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc17
  %indvars.iv58 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next59, %for.inc17 ]
  %k.050 = phi i32 [ undef, %for.body.lr.ph ], [ %k.1.lcssa, %for.inc17 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @b, i64 0, i64 %indvars.iv58
  %0 = load i32, ptr %arrayidx
  br i1 %cmp245, label %for.cond4.preheader.lr.ph, label %for.inc17

for.cond4.preheader.lr.ph:                        ; preds = %for.body
  %cmp543 = icmp sgt i32 %0, 0
  %wide.trip.count = zext i32 %0 to i64
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc14
  %indvars.iv53 = phi i64 [ 0, %for.cond4.preheader.lr.ph ], [ %indvars.iv.next54, %for.inc14 ]
  br i1 %cmp543, label %for.body6.lr.ph, label %for.inc14

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %1 = add nuw nsw i64 %indvars.iv58, %indvars.iv53
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %if.cont ]
  %2 = add nuw nsw i64 %1, %indvars.iv
  %arrayidx13 = getelementptr inbounds [100 x [100 x [3 x i32]]], ptr @a, i64 0, i64 %indvars.iv58, i64 %indvars.iv53, i64 %indvars.iv
  %arrayidx4 = getelementptr inbounds [100 x i32], ptr @c, i64 0, i64 %indvars.iv
  %ld0 = load i32, ptr %arrayidx4
  %cmp2 = icmp sgt i32 %ld0, 2
  br i1 %cmp2, label %if.then, label %if.cont

if.then:
  %cmp3 = icmp sgt i32 %ld0, 32
  br i1 %cmp3, label %for.end.split.loopexit, label %if.cont

if.cont:
  %3 = trunc i64 %2 to i32
  store i32 %3, ptr %arrayidx13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.inc14.loopexit, label %for.body6

for.inc14.loopexit:                               ; preds = %for.body6
  br label %for.inc14

for.inc14:                                        ; preds = %for.inc14.loopexit, %for.cond4.preheader
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next54, %wide.trip.count56
  br i1 %exitcond57.not, label %for.inc17.loopexit, label %for.cond4.preheader

for.inc17.loopexit:                               ; preds = %for.inc14
  %smax = call i32 @llvm.smax.i32(i32 %0, i32 0)
  br label %for.inc17

for.inc17:                                        ; preds = %for.inc17.loopexit, %for.body
  %k.1.lcssa = phi i32 [ %k.050, %for.body ], [ %smax, %for.inc17.loopexit ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next59, %wide.trip.count60
  br i1 %exitcond61.not, label %for.end19.loopexit, label %for.body

for.end19.loopexit:                               ; preds = %for.inc17
  %k.1.lcssa.lcssa = phi i32 [ %k.1.lcssa, %for.inc17 ]
  br label %for.end19

for.end.split.loopexit:                                ; preds = %for.inc6
  unreachable

for.end19:                                        ; preds = %for.end19.loopexit, %entry
  %k.0.lcssa = phi i32 [ undef, %entry ], [ %k.1.lcssa.lcssa, %for.end19.loopexit ]
  %sub = sub nsw i32 %n, %k.0.lcssa
  %idxprom20 = sext i32 %sub to i64
  %sub22 = sub nsw i32 %l, %n
  %idxprom23 = sext i32 %sub22 to i64
  %arrayidx25 = getelementptr inbounds [100 x [100 x [3 x i32]]], ptr @a, i64 0, i64 %idxprom20, i64 %idxprom23, i64 2
  %4 = load i32, ptr %arrayidx25
  ret i32 %4
}

declare i32 @llvm.smax.i32(i32, i32)
