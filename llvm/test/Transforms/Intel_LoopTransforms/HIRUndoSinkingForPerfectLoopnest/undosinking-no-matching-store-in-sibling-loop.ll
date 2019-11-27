; When sinked load inst cannot find matching store inst in the sibling loop, it will be hoisted out directly
;
; Source code:
;float a[100];
;float b[100];
;float c[100];
;
;void multiply() {
;  for(int k = 0; k < 100; k++){
;    for(int i = 0; i < 100; i++){
;      b[i] *= a[i];
;    }
;
;    for (int i = 0; i < 100; i++) {
;      for (int j = 0; j < 80; j++) {
;        c[i] += a[2 * i + j] * b[j];
;      }
;    }
;  }
;}
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-undo-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest -print-after=hir-undo-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-undo-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: multiply
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   %mul = (@a)[0][i2]  *  (@b)[0][i2];
; CHECK:           |   |   (@b)[0][i2] = %mul;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   |   %add2450 = (@c)[0][i2];
; CHECK:           |   |   |   %mul21 = (@a)[0][2 * i2 + i3]  *  (@b)[0][i3];
; CHECK:           |   |   |   %add2450 = %add2450  +  %mul21;
; CHECK:           |   |   |   (@c)[0][i2] = %add2450;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;*** IR Dump After HIR Undo Sinking For Perfect Loopnest ***
;Function: multiply
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   %mul = (@a)[0][i2]  *  (@b)[0][i2];
; CHECK:           |   |   (@b)[0][i2] = %mul;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |      %add2450 = (@c)[0][i2];
; CHECK:           |   |   + DO i3 = 0, 79, 1   <DO_LOOP>
; CHECK:           |   |   |   %mul21 = (@a)[0][2 * i2 + i3]  *  (@b)[0][i3];
; CHECK:           |   |   |   %add2450 = %add2450  +  %mul21;
; CHECK:           |   |   + END LOOP
; CHECK:           |   |      (@c)[0][i2] = %add2450;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@d = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

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
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx6, align 4, !tbaa !2
  %mul = fmul float %0, %1
  store float %mul, float* %arrayidx6, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond12.preheader.preheader, label %for.body4

for.cond12.preheader.preheader:                   ; preds = %for.body4
  br label %for.cond12.preheader

for.cond12.preheader:                             ; preds = %for.cond12.preheader.preheader, %for.cond.cleanup14
  %indvars.iv57 = phi i64 [ %indvars.iv.next58, %for.cond.cleanup14 ], [ 0, %for.cond12.preheader.preheader ]
  %2 = shl nuw nsw i64 %indvars.iv57, 1
  %arrayidx23 = getelementptr inbounds [100 x float], [100 x float]* @c, i64 0, i64 %indvars.iv57, !intel-tbaa !2
  %arrayidx23.promoted = load float, float* %arrayidx23, align 4, !tbaa !2
  br label %for.body15

for.cond.cleanup10:                               ; preds = %for.cond.cleanup14
  %inc32 = add nuw nsw i32 %k.052, 1
  %exitcond61 = icmp eq i32 %inc32, 100
  br i1 %exitcond61, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup14:                               ; preds = %for.body15
  %add24.lcssa = phi float [ %add24, %for.body15 ]
  store float %add24.lcssa, float* %arrayidx23, align 4, !tbaa !2
  %indvars.iv.next58 = add nuw nsw i64 %indvars.iv57, 1
  %exitcond60 = icmp eq i64 %indvars.iv.next58, 100
  br i1 %exitcond60, label %for.cond.cleanup10, label %for.cond12.preheader

for.body15:                                       ; preds = %for.body15, %for.cond12.preheader
  %indvars.iv53 = phi i64 [ 0, %for.cond12.preheader ], [ %indvars.iv.next54, %for.body15 ]
  %add2450 = phi float [ %arrayidx23.promoted, %for.cond12.preheader ], [ %add24, %for.body15 ]
  %3 = add nuw nsw i64 %indvars.iv53, %2
  %arrayidx18 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %3, !intel-tbaa !2
  %4 = load float, float* %arrayidx18, align 4, !tbaa !2
  %arrayidx20 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv53, !intel-tbaa !2
  %5 = load float, float* %arrayidx20, align 4, !tbaa !2
  %mul21 = fmul float %4, %5
  %add24 = fadd float %add2450, %mul21
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next54, 80
  br i1 %exitcond56, label %for.cond.cleanup14, label %for.body15
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
