; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,hir-loop-fusion,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Test checks that we can fuse loops when collapsed refs have same number of levels and same indices.

; HIR before optimization:
;<0>          BEGIN REGION { }
;<65>               + DO i1 = 0, 19, 1   <DO_LOOP>
;<66>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<67>               |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;<9>                |   |   |   (@a)[0][i1][i2][i3] = %n;
;<67>               |   |   + END LOOP
;<66>               |   + END LOOP
;<65>               + END LOOP
;<65>
;<32>               %s.065 = 0;
;<68>
;<68>               + DO i1 = 0, 19, 1   <DO_LOOP>
;<69>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<70>               |   |   + DO i3 = 0, 19, 1   <DO_LOOP>
;<43>               |   |   |   %s.065 = (@a)[0][i1][i2][i3]  +  %s.065;
;<70>               |   |   + END LOOP
;<69>               |   + END LOOP
;<68>               + END LOOP
;<0>          END REGION

; HIR after fusion
; CHECK:     BEGIN REGION { modified }
; CHECK:           %s.065 = 0;
; CHECK:           + DO i1 = 0, 7999, 1   <DO_LOOP>  <MAX_TC_EST = 8000>  <LEGAL_MAX_TC = 8000>
; CHECK-NEXT:      |   (@a)[0][0][0][i1] = %n;
; CHECK-NEXT:      |   %s.065 = (@a)[0][0][0][i1]  +  %s.065;
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [20 x [20 x [20 x i32]]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo(i32 noundef %n) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc14
  %indvars.iv69 = phi i64 [ 0, %entry ], [ %indvars.iv.next70, %for.inc14 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc11
  %indvars.iv66 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next67, %for.inc11 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [20 x [20 x [20 x i32]]], ptr @a, i64 0, i64 %indvars.iv69, i64 %indvars.iv66, i64 %indvars.iv
  store i32 %n, ptr %arrayidx10, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond.not, label %for.inc11, label %for.body6

for.inc11:                                        ; preds = %for.body6
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond68.not = icmp eq i64 %indvars.iv.next67, 20
  br i1 %exitcond68.not, label %for.inc14, label %for.cond4.preheader

for.inc14:                                        ; preds = %for.inc11
  %indvars.iv.next70 = add nuw nsw i64 %indvars.iv69, 1
  %exitcond71.not = icmp eq i64 %indvars.iv.next70, 20
  br i1 %exitcond71.not, label %for.cond20.preheader.preheader, label %for.cond1.preheader

for.cond20.preheader.preheader:                   ; preds = %for.inc14
  br label %for.cond20.preheader

for.cond20.preheader:                             ; preds = %for.cond20.preheader.preheader, %for.inc38
  %indvars.iv78 = phi i64 [ %indvars.iv.next79, %for.inc38 ], [ 0, %for.cond20.preheader.preheader ]
  %s.065 = phi i32 [ %add.lcssa.lcssa, %for.inc38 ], [ 0, %for.cond20.preheader.preheader ]
  br label %for.cond23.preheader

for.cond23.preheader:                             ; preds = %for.cond20.preheader, %for.inc35
  %indvars.iv75 = phi i64 [ 0, %for.cond20.preheader ], [ %indvars.iv.next76, %for.inc35 ]
  %s.163 = phi i32 [ %s.065, %for.cond20.preheader ], [ %add.lcssa, %for.inc35 ]
  br label %for.body25

for.body25:                                       ; preds = %for.cond23.preheader, %for.body25
  %indvars.iv72 = phi i64 [ 0, %for.cond23.preheader ], [ %indvars.iv.next73, %for.body25 ]
  %s.261 = phi i32 [ %s.163, %for.cond23.preheader ], [ %add, %for.body25 ]
  %arrayidx31 = getelementptr inbounds [20 x [20 x [20 x i32]]], ptr @a, i64 0, i64 %indvars.iv78, i64 %indvars.iv75, i64 %indvars.iv72
  %0 = load i32, ptr %arrayidx31, align 4
  %add = add nsw i32 %0, %s.261
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond74.not = icmp eq i64 %indvars.iv.next73, 20
  br i1 %exitcond74.not, label %for.inc35, label %for.body25

for.inc35:                                        ; preds = %for.body25
  %add.lcssa = phi i32 [ %add, %for.body25 ]
  %indvars.iv.next76 = add nuw nsw i64 %indvars.iv75, 1
  %exitcond77.not = icmp eq i64 %indvars.iv.next76, 20
  br i1 %exitcond77.not, label %for.inc38, label %for.cond23.preheader

for.inc38:                                        ; preds = %for.inc35
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.inc35 ]
  %indvars.iv.next79 = add nuw nsw i64 %indvars.iv78, 1
  %exitcond80.not = icmp eq i64 %indvars.iv.next79, 20
  br i1 %exitcond80.not, label %for.end40, label %for.cond20.preheader

for.end40:                                        ; preds = %for.inc38
  %add.lcssa.lcssa.lcssa = phi i32 [ %add.lcssa.lcssa, %for.inc38 ]
  ret i32 %add.lcssa.lcssa.lcssa
}
