; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check DO i3, DO i4 are collapsed.

;   BEGIN REGION { }
;         + DO i1 = 0, 2, 1   <DO_LOOP>
;         |   + DO i2 = 0, 15, 1   <DO_LOOP>
;         |   |   + DO i3 = 0, 7, 1   <DO_LOOP>
;         |   |   |   + DO i4 = 0, 7, 1   <DO_LOOP>
;         |   |   |   |   %and = (@x)[0][8 * i1 + i3][i4]  &  (@x)[0][128 * i1 + 8 * i2 + i3][i4];
;         |   |   |   |   (@y)[0][128 * i1 + 8 * i2 + i3][i4] = %and;
;         |   |   |   + END LOOP
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP
;   END REGION

;   BEGIN REGION { modified }
;         + DO i1 = 0, 2, 1   <DO_LOOP>
;         |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:  |   |   + DO i3 = 0, 63, 1   <DO_LOOP>
; CHECK:  |   |   |   %and = (@x)[0][8 * i1][i3]  &  (@x)[0][128 * i1 + 8 * i2][i3];
; CHECK:  |   |   |   (@y)[0][128 * i1 + 8 * i2][i3] = %and;
; CHECK:  |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP
;   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local local_unnamed_addr global [1024 x [8 x i64]] zeroinitializer, align 16
@y = dso_local local_unnamed_addr global [1024 x [8 x i64]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv73 = phi i64 [ 0, %entry ], [ %indvars.iv.next74, %for.cond.cleanup3 ]
  %0 = shl nsw i64 %indvars.iv73, 7
  %1 = shl nsw i64 %indvars.iv73, 3
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %indvars.iv68 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next69, %for.cond.cleanup7 ]
  %2 = shl nsw i64 %indvars.iv68, 3
  %3 = add nuw nsw i64 %2, %0
  br label %for.cond9.preheader

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond77 = icmp eq i64 %indvars.iv.next74, 3
  br i1 %exitcond77, label %for.cond.cleanup, label %for.cond1.preheader

for.cond9.preheader:                              ; preds = %for.cond.cleanup11, %for.cond5.preheader
  %indvars.iv63 = phi i64 [ 0, %for.cond5.preheader ], [ %indvars.iv.next64, %for.cond.cleanup11 ]
  %4 = add nuw nsw i64 %3, %indvars.iv63
  %5 = add nuw nsw i64 %indvars.iv63, %1
  br label %for.body12

for.cond.cleanup7:                                ; preds = %for.cond.cleanup11
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond72 = icmp eq i64 %indvars.iv.next69, 16
  br i1 %exitcond72, label %for.cond.cleanup3, label %for.cond5.preheader

for.cond.cleanup11:                               ; preds = %for.body12
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next64, 8
  br i1 %exitcond67, label %for.cond.cleanup7, label %for.cond9.preheader

for.body12:                                       ; preds = %for.body12, %for.cond9.preheader
  %indvars.iv = phi i64 [ 0, %for.cond9.preheader ], [ %indvars.iv.next, %for.body12 ]
  %arrayidx16 = getelementptr inbounds [1024 x [8 x i64]], ptr @x, i64 0, i64 %4, i64 %indvars.iv
  %6 = load i64, ptr %arrayidx16, align 8
  %arrayidx22 = getelementptr inbounds [1024 x [8 x i64]], ptr @x, i64 0, i64 %5, i64 %indvars.iv
  %7 = load i64, ptr %arrayidx22, align 8
  %and = and i64 %7, %6
  %arrayidx30 = getelementptr inbounds [1024 x [8 x i64]], ptr @y, i64 0, i64 %4, i64 %indvars.iv
  store i64 %and, ptr %arrayidx30, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond, label %for.cond.cleanup11, label %for.body12
}
