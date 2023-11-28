; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that we distribute the following loop with independent refs that are vectorizable
; in multiple separate loopnests.

;           BEGIN REGION { }
;                 + DO i1 = 0, 98, 1
;                 |   (%M)[i1] = %N1;
;                 |
;                 |   + DO i2 = 0, zext.i32.i64(%N1) + -1, 1
;                 |   |   (@B)[0][i1][i2] = i2;
;                 |   + END LOOP
;                 |
;                 |
;                 |   + DO i2 = 0, zext.i32.i64(%N2) + -1, 1
;                 |   |   (@C)[0][i1][i2] = -1 * i2;
;                 |   + END LOOP
;                 + END LOOP
;           END REGION


; CHECK:    BEGIN REGION { }
; CHECK:          + DO i1 = 0, 98, 1
;                 |   (@A)[0][i1] = %N1;
;                 + END LOOP
;
;
; CHECK:          + DO i1 = 0, 98, 1
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%N1) + -1, 1
;                 |   |   (@B)[0][i1][i2] = i2;
;                 |   + END LOOP
;                 + END LOOP
;
;
; CHECK:          + DO i1 = 0, 98, 1
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%N2) + -1, 1
;                 |   |   (@C)[0][i1][i2] = -1 * i2;
;                 |   + END LOOP
;                 + END LOOP
; CHECK:    END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local void @sub(i32 noundef %N1, i32 noundef %N2, ptr nocapture noundef readnone %M, ptr nocapture noundef readnone %N, ptr nocapture noundef readnone %K) local_unnamed_addr #0 {
entry:
  %cmp235 = icmp sgt i32 %N1, 0
  %cmp1137 = icmp sgt i32 %N2, 0
  %wide.trip.count = zext i32 %N1 to i64
  %wide.trip.count43 = zext i32 %N2 to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup12
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup12
  %indvars.iv45 = phi i64 [ 0, %entry ], [ %indvars.iv.next46, %for.cond.cleanup12 ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv45
  store i32 %N1, ptr %arrayidx, align 4
  br i1 %cmp235, label %for.body4.preheader, label %for.cond10.preheader

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.cond10.preheader.loopexit:                    ; preds = %for.body4
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond10.preheader.loopexit, %for.body
  br i1 %cmp1137, label %for.body13.preheader, label %for.cond.cleanup12

for.body13.preheader:                             ; preds = %for.cond10.preheader
  br label %for.body13

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx8 = getelementptr inbounds [1000 x [1000 x i32]], ptr @B, i64 0, i64 %indvars.iv45, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond10.preheader.loopexit, label %for.body4

for.cond.cleanup12.loopexit:                      ; preds = %for.body13
  br label %for.cond.cleanup12

for.cond.cleanup12:                               ; preds = %for.cond.cleanup12.loopexit, %for.cond10.preheader
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond47.not = icmp eq i64 %indvars.iv.next46, 99
  br i1 %exitcond47.not, label %for.cond.cleanup, label %for.body

for.body13:                                       ; preds = %for.body13.preheader, %for.body13
  %indvars.iv40 = phi i64 [ %indvars.iv.next41, %for.body13 ], [ 0, %for.body13.preheader ]
  %arrayidx17 = getelementptr inbounds [1000 x [1000 x i32]], ptr @C, i64 0, i64 %indvars.iv45, i64 %indvars.iv40
  %1 = trunc i64 %indvars.iv40 to i32
  %2 = sub i32 0, %1
  store i32 %2, ptr %arrayidx17, align 4
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %exitcond44.not = icmp eq i64 %indvars.iv.next41, %wide.trip.count43
  br i1 %exitcond44.not, label %for.cond.cleanup12.loopexit, label %for.body13
}