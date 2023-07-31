; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we are able to do DSE store of (@A)[0][i1][i2] by forward
; subtituting it in the load. We should be able to ignore store to
; (@A)[0][i1 + 1][i2] during analysis.

; CHECK: Dump Before

; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT:   |   |   (@A)[0][i1 + 1][i2] = 55;
; CHECK-NEXT:   |   |   (@A)[0][i1][i2] = 5;
; CHECK-NEXT:   |   |   %ld = (@A)[0][i1][i2];
; CHECK-NEXT:   |   + END LOOP
; CHECK-NEXT:   + END LOOP

; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT:   |   |   (@A)[0][i1][i2] = 10;
; CHECK-NEXT:   |   + END LOOP
; CHECK-NEXT:   + END LOOP
; CHECK-NEXT:      %ld.out = %ld;

; CHECK:        ret %ld.out;


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }

; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT:   |   |   (@A)[0][i1 + 1][i2] = 55;
; CHECK-NEXT:   |   |   %ld = 5;
; CHECK-NEXT:   |   + END LOOP
; CHECK-NEXT:   + END LOOP

; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:   |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT:   |   |   (@A)[0][i1][i2] = 10;
; CHECK-NEXT:   |   + END LOOP
; CHECK-NEXT:   + END LOOP
; CHECK-NEXT:      %ld.out = %ld;

; CHECK:        ret %ld.out;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local i32 @foo(i32 %n) {
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
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv49, i64 %indvars.iv46
  %iv.add = add i64 %indvars.iv49, 1
  %arrayidx55 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %iv.add, i64 %indvars.iv46
  store i32 55, ptr %arrayidx55, align 4
  store i32 5, ptr %arrayidx5, align 4
  %ld = load i32, ptr %arrayidx5, align 4
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 4
  br i1 %exitcond48, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %ld.lcssa = phi i32 [ %ld, %for.body3 ]
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next50, %wide.trip.count51
  br i1 %exitcond52, label %for.cond12.preheader.preheader, label %for.cond1.preheader

for.cond12.preheader.preheader:                   ; preds = %for.inc6
  %ld.lcssa1 = phi i32 [ %ld.lcssa, %for.inc6 ]
  %wide.trip.count = sext i32 %n to i64
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %for.inc22, %for.cond12.preheader.preheader
  %indvars.iv43 = phi i64 [ 0, %for.cond12.preheader.preheader ], [ %indvars.iv.next44, %for.inc22 ]
  br label %for.body14

for.body14:                                       ; preds = %for.body14, %for.cond12.preheader
  %indvars.iv = phi i64 [ 0, %for.cond12.preheader ], [ %indvars.iv.next, %for.body14 ]
  %arrayidx18 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %indvars.iv43, i64 %indvars.iv
  store i32 10, ptr %arrayidx18, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.inc22, label %for.body14

for.inc22:                                        ; preds = %for.body14
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next44, %wide.trip.count
  br i1 %exitcond45, label %for.end24.loopexit, label %for.cond12.preheader

for.end24.loopexit:                               ; preds = %for.inc22
  %ld.lcssa2 = phi i32 [ %ld.lcssa1, %for.inc22 ]
  br label %for.end24

for.end24:                                        ; preds = %for.end24.loopexit, %entry
  %ld.out = phi i32 [ %ld.lcssa2, %for.end24.loopexit ], [ 0, %entry ]
  ret i32 %ld.out
}

