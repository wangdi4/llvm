; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -S -aa-pipeline="basic-aa" -disable-output -disable-hir-pragma-bailout < %s 2>&1 | FileCheck %s

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   %t1 = (@A)[0][i2];
;       |   |   %t2 = (@B)[0][i2];
;       |   |   %add = %t2  +  %t1;
;       |   |   (@B)[0][i2] = %add;
;       |   |   (@C)[0][i2] = %t1;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   %t1 = (@A)[0][i2];
; CHECK:       |   |   %t2 = (@B)[0][i2];
; CHECK:       |   |   %add = %t2  +  %t1;
; CHECK:       |   |   (@B)[0][i2] = %add;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP

; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   |   %t1 = (@A)[0][i2];
; CHECK:       |   |   (@C)[0][i2] = %t1;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  %0 = trunc i64 %indvars.iv31 to i32
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond33 = icmp eq i64 %indvars.iv.next32, 100
  br i1 %exitcond33, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv
  %t1 = load float, ptr %arrayidx, align 4
  %arrayidx7 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv
  %t2 = load float, ptr %arrayidx7, align 4
  %add = fadd float %t2, %t1
  store float %add, ptr %arrayidx7, align 4
  %arrayidx12 = getelementptr inbounds [100 x float], ptr @C, i64 0, i64 %indvars.iv
  store float %t1, ptr %arrayidx12, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4, !llvm.loop !1
}

!1 = !{!1, !2}
!2 = !{!"intel.loop.distribute.loopnest.enable", i32 1}

