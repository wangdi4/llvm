; RUN: opt -hir-create-function-level-region -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -hir-create-function-level-region -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that we are able to do DSE in independent sibling loopnests
; Dump Before

; CHECK: BEGIN REGION { }
; CHECK: if (%n > 0)
; CHECK: {
; CHECK:   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:   |   |   (@A)[0][i1][i2] = 5;
; CHECK:   |   + END LOOP
; CHECK:   + END LOOP

; CHECK:   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:   |   |   (@A)[0][i1][i2] = 10;
; CHECK:   |   + END LOOP
; CHECK:   + END LOOP
; CHECK: }
; CHECK: ret ;
; CHECK: END REGION


; Dump After

; CHECK: BEGIN REGION { modified }
; CHECK: if (%n > 0)
; CHECK: {
; CHECK:   + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:   |   |   (@A)[0][i1][i2] = 10;
; CHECK:   |   + END LOOP
; CHECK:   + END LOOP
; CHECK: }
; CHECK: ret ;
; CHECK: END REGION


source_filename = "sibling-dse.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) {
entry:
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.cond1.preheader.preheader, label %for.end24

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count51 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %for.cond1.preheader.preheader
  %indvars.iv49 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next50, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv46 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next47, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv49, i64 %indvars.iv46
  store i32 5, i32* %arrayidx5, align 4
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 4
  br i1 %exitcond48, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next50, %wide.trip.count51
  br i1 %exitcond52, label %for.cond12.preheader.preheader, label %for.cond1.preheader

for.cond12.preheader.preheader:                   ; preds = %for.inc6
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %for.inc22, %for.cond12.preheader.preheader
  %indvars.iv43 = phi i64 [ 0, %for.cond12.preheader.preheader ], [ %indvars.iv.next44, %for.inc22 ]
  br label %for.body14

for.body14:                                       ; preds = %for.body14, %for.cond12.preheader
  %indvars.iv = phi i64 [ 0, %for.cond12.preheader ], [ %indvars.iv.next, %for.body14 ]
  %arrayidx18 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store i32 10, i32* %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc22, label %for.body14

for.inc22:                                        ; preds = %for.body14
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, %wide.trip.count
  br i1 %exitcond45, label %for.end24.loopexit, label %for.cond12.preheader

for.end24.loopexit:                               ; preds = %for.inc22
  br label %for.end24

for.end24:                                        ; preds = %for.end24.loopexit, %entry
  ret void
}

