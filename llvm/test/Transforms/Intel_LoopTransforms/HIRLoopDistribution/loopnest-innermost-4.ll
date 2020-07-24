; RUN: opt -hir-ssa-deconstruction -hir-loop-distribute-loopnest -S -print-after=hir-loop-distribute-loopnest -disable-output -disable-hir-pragma-bailout < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -S -aa-pipeline="basic-aa" -disable-output -disable-hir-pragma-bailout < %s 2>&1 | FileCheck %s

; Check that loop will distribute into two chunks by hir-loop-distribute-loopnest.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   %0 = sitofp.i64.float(i2);
;       |   |   %conv5 = %0  +  1.000000e+01;
;       |   |   (@p)[0][i2] = %conv5;
;       |   |   (@q)[0][i2] = %conv5;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   %0 = sitofp.i64.float(i2);
; CHECK:       |   |   %conv5 = %0  +  1.000000e+01;
; CHECK:       |   |   (@p)[0][i2] = %conv5;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
;
;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   %0 = sitofp.i64.float(i2);
; CHECK:       |   |   %conv5 = %0  +  1.000000e+01;
; CHECK:       |   |   (@q)[0][i2] = %conv5;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@p = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@q = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@p1 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@q1 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %i.027 = phi i32 [ 0, %entry ], [ %inc16, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc16 = add nuw nsw i32 %i.027, 1
  %exitcond28 = icmp eq i32 %inc16, 100
  br i1 %exitcond28, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @p, i64 0, i64 %indvars.iv
  %arrayidx1 = getelementptr inbounds [100 x float], [100 x float]* @p1, i64 0, i64 %indvars.iv
  ;%0 = load float, float* %arrayidx1, align 4
  %0 = sitofp i64 %indvars.iv to float
  %conv5 = fadd float %0, 1.000000e+01
  store float %conv5, float* %arrayidx, align 4
  %arrayidx9 = getelementptr inbounds [100 x float], [100 x float]* @q, i64 0, i64 %indvars.iv
  store float %conv5, float* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4, !llvm.loop !1
}

!1 = !{!1, !2}
!2 = !{!"intel.loop.distribute.loopnest.enable", i32 1}

