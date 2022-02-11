; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-fusion -hir-create-function-level-region -debug-only=hir-loop-fusion -print-after=hir-temp-cleanup  -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region -debug-only=hir-loop-fusion -print-after=hir-temp-cleanup < %s 2>&1 | FileCheck %s

; Test checks that inner and outer loops are marked vectorizable and non-vectorizable respectively during fusion.

; CHECK: Dump After HIR{{.*}}Temp{{.*}}Cleanup
; CHECK: <{{[0-9]+}}>  BEGIN REGION { }
; CHECK: <[[L1:[0-9]+]]>  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK: <[[L11:[0-9]+]]> |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        <{{[0-9]+}}>     |   |   (@a)[0][i1][i2] = 5.000000e+00;
;        <{{[0-9]+}}>     |   + END LOOP
;        <{{[0-9]+}}>     + END LOOP

; CHECK: <[[L2:[0-9]+]]>  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        <{{[0-9]+}}>     |   %conv = sitofp.i32.float(i1);
;        <{{[0-9]+}}>     |   (@b)[0][i1] = %conv;
;        <{{[0-9]+}}>     + END LOOP

; CHECK: <[[L3:[0-9]+]]>  + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
; CHECK: <[[L31:[0-9]+]]> |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>  <LEGAL_MAX_TC = 2147483647>
;        <{{[0-9]+}}>     |   |   %conv30 = (@a)[0][i1][i2]  +  3.000000e+00;
;        <{{[0-9]+}}>     |   |   (@a)[0][i1][i2] = %conv30;
;        <{{[0-9]+}}>     |   + END LOOP
;        <{{[0-9]+}}>     + END LOOP
;
;        <{{[0-9]+}}>  END REGION

; CHECK: Final Fusion Nodes:
; CHECK: { [[L1]] [[L3]] }
; CHECK-NOT: V
; CHECK: While HIR Loop Fusion:
; CHECK: Final Fusion Nodes:
; CHECK: { [[L11]] [[L31]] }V
; CHECK: { [[L2]] }V
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [100 x [100 x float]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @sub(i32 %n) local_unnamed_addr {
entry:
  %cmp263 = icmp sgt i32 %n, 0
  br i1 %cmp263, label %for.cond1.preheader.preheader, label %for.end36

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count8183 = zext i32 %n to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc6, %for.cond1.preheader.preheader
  %indvars.iv79 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next80, %for.inc6 ]
  br label %for.body3

for.body11.preheader:                             ; preds = %for.inc6
  br label %for.body11

for.body3:                                        ; preds = %for.body3.preheader, %for.body3
  %indvars.iv75 = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next76, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @a, i64 0, i64 %indvars.iv79, i64 %indvars.iv75
  store float 5.000000e+00, float* %arrayidx5, align 4
  %indvars.iv.next76 = add nuw nsw i64 %indvars.iv75, 1
  %exitcond78.not = icmp eq i64 %indvars.iv.next76, %wide.trip.count8183
  br i1 %exitcond78.not, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next80 = add nuw nsw i64 %indvars.iv79, 1
  %exitcond82.not = icmp eq i64 %indvars.iv.next80, %wide.trip.count8183
  br i1 %exitcond82.not, label %for.body11.preheader, label %for.body3.preheader

for.cond21.preheader.preheader:                   ; preds = %for.body11
  br label %for.body24.preheader

for.body11:                                       ; preds = %for.body11.preheader, %for.body11
  %indvars.iv71 = phi i64 [ 0, %for.body11.preheader ], [ %indvars.iv.next72, %for.body11 ]
  %0 = trunc i64 %indvars.iv71 to i32
  %conv = sitofp i32 %0 to float
  %arrayidx13 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv71
  store float %conv, float* %arrayidx13, align 4
  %indvars.iv.next72 = add nuw nsw i64 %indvars.iv71, 1
  %exitcond74.not = icmp eq i64 %indvars.iv.next72, %wide.trip.count8183
  br i1 %exitcond74.not, label %for.cond21.preheader.preheader, label %for.body11

for.body24.preheader:                             ; preds = %for.inc34, %for.cond21.preheader.preheader
  %indvars.iv67 = phi i64 [ 0, %for.cond21.preheader.preheader ], [ %indvars.iv.next68, %for.inc34 ]
  br label %for.body24

for.body24:                                       ; preds = %for.body24.preheader, %for.body24
  %indvars.iv = phi i64 [ 0, %for.body24.preheader ], [ %indvars.iv.next, %for.body24 ]
  %arrayidx28 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* @a, i64 0, i64 %indvars.iv67, i64 %indvars.iv
  %1 = load float, float* %arrayidx28, align 4
  %conv30 = fadd fast float %1, 3.000000e+00
  store float %conv30, float* %arrayidx28, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count8183
  br i1 %exitcond.not, label %for.inc34, label %for.body24

for.inc34:                                        ; preds = %for.body24
  %indvars.iv.next68 = add nuw nsw i64 %indvars.iv67, 1
  %exitcond70.not = icmp eq i64 %indvars.iv.next68, %wide.trip.count8183
  br i1 %exitcond70.not, label %for.end36.loopexit, label %for.body24.preheader

for.end36.loopexit:                               ; preds = %for.inc34
  br label %for.end36

for.end36:                                        ; preds = %for.end36.loopexit, %entry
  ret void
}

