; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -S -aa-pipeline="basic-aa" -disable-output -disable-hir-pragma-bailout < %s 2>&1 | FileCheck %s

; Check that loop is distributed into two i1-i2 loopnests and i2 loop
; preheader is moved to the second chunk.

; Previously, after i2 loop distribution preheader was kept with the first
; chunk even though it is unused.
; This was preventing i1 loop from getting distributed because of %add -> %add
; flow edge between the two i2 loops.

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |      %add = %t  +  1.000000e+01;
; |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; |   |   %0 = (@p)[0][i2];
; |   |   %conv5 = %0  +  1.000000e+01;
; |   |   (@p)[0][i2] = %conv5;
; |   |   %1 = (@q)[0][i2];
; |   |   %conv12 = %1  +  %add;
; |   |   (@q)[0][i2] = %conv12;
; |   + END LOOP
; + END LOOP

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NOT: %add
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %0 = (@p)[0][i2];
; CHECK: |   |   %conv5 = %0  +  1.000000e+01;
; CHECK: |   |   (@p)[0][i2] = %conv5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |      %add = %t  +  1.000000e+01;
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   |   %1 = (@q)[0][i2];
; CHECK: |   |   %conv12 = %1  +  %add;
; CHECK: |   |   (@q)[0][i2] = %conv12;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@p = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@q = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@p1 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@q1 = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i64 %n, float %t) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %i.027 = phi i32 [ 0, %entry ], [ %inc16, %for.cond.cleanup3 ]
  %ztt = icmp sgt i64 %n, 0
  br i1 %ztt, label %for.body4.pre, label %for.cond.cleanup3

for.body4.pre:
  %add = fadd float %t, 1.000000e+01
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body4.pre
  %indvars.iv = phi i64 [ 0, %for.body4.pre ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @p, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %conv5 = fadd float %0, 1.000000e+01
  store float %conv5, ptr %arrayidx, align 4
  %arrayidx9 = getelementptr inbounds [100 x float], ptr @q, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx9, align 4
  %conv12 = fadd float %1, %add
  store float %conv12, ptr %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.exit, label %for.body4, !llvm.loop !1

for.exit:
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.body4
  %inc16 = add nuw nsw i32 %i.027, 1
  %exitcond28 = icmp eq i32 %inc16, 100
  br i1 %exitcond28, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void
}

!1 = !{!1, !2}
!2 = !{!"intel.loop.distribute.loopnest.enable", i32 1}

