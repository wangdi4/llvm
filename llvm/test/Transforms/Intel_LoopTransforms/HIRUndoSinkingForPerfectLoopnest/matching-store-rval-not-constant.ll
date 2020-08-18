; This is one of those sibling loop cases for undoing sinking after loop distribution.
; We can find a matching store (@c)[0][i2] = %mul; for %add2453 = (@c)[0][i2]; in the sibling loop.
; When matching store's Rval is not a constant, we directly hoist out the sinked load inst
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-undo-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest -print-after=hir-undo-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-undo-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: multiply
;
;<0>          BEGIN REGION { }
;<55>               + DO i1 = 0, 99, 1   <DO_LOOP>
;<56>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<7>                |   |   %mul = (@d)[0][i2]  *  5.000000e+00;
;<9>                |   |   (@c)[0][i2] = %mul;
;<56>               |   + END LOOP
;<56>               |
;<57>               |
;<57>               |   + DO i2 = 0, 99, 1   <DO_LOOP>
;<58>               |   |   + DO i3 = 0, 79, 1   <DO_LOOP>
;<22>               |   |   |   %add2453 = (@c)[0][i2];
;<32>               |   |   |   %mul21 = (@a)[0][2 * i2 + i3]  *  (@b)[0][i3];
;<33>               |   |   |   %add2453 = %add2453  +  %mul21;
;<41>               |   |   |   (@c)[0][i2] = %add2453;
;<58>               |   |   + END LOOP
;<57>               |   + END LOOP
;<55>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;Function: multiply
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   %mul = (@d)[0][i2]  *  5.000000e+00;
; CHECK:           |   |   (@c)[0][i2] = %mul;
; CHECK:           |   + END LOOP
;                  |
;                  |
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |      %add2453 = (@c)[0][i2];
; CHECK:           |   |   + DO i3 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   |   %mul21 = (@a)[0][2 * i2 + i3]  *  (@b)[0][i3];
; CHECK:           |   |   |   %add2453 = %add2453  +  %mul21;
; CHECK:           |   |   + END LOOP
; CHECK:           |   |      (@c)[0][i2] = %add2453;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @multiply() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup10, %entry
  %k.052 = phi i32 [ 0, %entry ], [ %inc32, %for.cond.cleanup10 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup10
  ret void

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @d, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %mul = fmul float %0, 5.000000e+00
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store float %mul, float* %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond12.preheader.preheader, label %for.body4

for.cond12.preheader.preheader:                   ; preds = %for.body4
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %for.cond12.preheader.preheader, %for.cond.cleanup14
  %indvars.iv58 = phi i64 [ %indvars.iv.next59, %for.cond.cleanup14 ], [ 0, %for.cond12.preheader.preheader ]
  %1 = shl nuw nsw i64 %indvars.iv58, 1
  %arrayidx23 = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %indvars.iv58, !intel-tbaa !2
  %arrayidx23.promoted = load float, float* %arrayidx23, align 4, !tbaa !2
  br label %for.body15

for.cond.cleanup10:                               ; preds = %for.cond.cleanup14
  %inc32 = add nuw nsw i32 %k.052, 1
  %exitcond62.not = icmp eq i32 %inc32, 100
  br i1 %exitcond62.not, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup14:                               ; preds = %for.body15
  %add24.lcssa = phi float [ %add24, %for.body15 ]
  store float %add24.lcssa, float* %arrayidx23, align 4, !tbaa !2
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next59, 100
  br i1 %exitcond61.not, label %for.cond.cleanup10, label %for.cond12.preheader

for.body15:                                       ; preds = %for.body15, %for.cond12.preheader
  %indvars.iv54 = phi i64 [ 0, %for.cond12.preheader ], [ %indvars.iv.next55, %for.body15 ]
  %add2453 = phi float [ %arrayidx23.promoted, %for.cond12.preheader ], [ %add24, %for.body15 ]
  %2 = add nuw nsw i64 %indvars.iv54, %1
  %arrayidx18 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %2, !intel-tbaa !2
  %3 = load float, float* %arrayidx18, align 4, !tbaa !2
  %arrayidx20 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv54, !intel-tbaa !2
  %4 = load float, float* %arrayidx20, align 4, !tbaa !2
  %mul21 = fmul float %3, %4
  %add24 = fadd float %add2453, %mul21
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next55, 80
  br i1 %exitcond57.not, label %for.cond.cleanup14, label %for.body15
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
