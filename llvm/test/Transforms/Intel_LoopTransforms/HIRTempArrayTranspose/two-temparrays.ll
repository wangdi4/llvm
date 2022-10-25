; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose" -print-after=hir-temp-array-transpose -hir-temp-array-transpose-tc=2 -disable-output 2>&1 | FileCheck %s

; Check that Temp Array Transpose occurs twice for Refs %a and %b

; HIR before
;       BEGIN REGION { }
;             + DO i1 = 0, 3, 1   <DO_LOOP>
;             |   + DO i2 = 0, 3, 1   <DO_LOOP>
;             |   |   %tmp.038 = 0.000000e+00;
;             |   |
;             |   |   + DO i3 = 0, 3, 1   <DO_LOOP>
;             |   |   |   %mul11 = (%b)[i2 + sext.i32.i64(%0) * i3]  *  (%a)[i1 + sext.i32.i64(%0) * i3];
;             |   |   |   %tmp.038 = %mul11  +  %tmp.038;
;             |   |   + END LOOP
;             |   |
;             |   |   %sub = (%result)[sext.i32.i64(%0) * i1 + i2]  -  %tmp.038;
;             |   |   (%result)[sext.i32.i64(%0) * i1 + i2] = %sub;
;             |   + END LOOP
;             + END LOOP
;       END REGION

; HIR After

; CHECK:  BEGIN REGION { modified }
; CHECK:      %call = @llvm.stacksave();
; CHECK:      %TranspTmpArr = alloca 64;
; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   |   (%TranspTmpArr)[i1][i2] = (%a)[i2][i1];
; CHECK:      |   + END LOOP
; CHECK:      + END LOOP
; CHECK:      %call3 = @llvm.stacksave();
; CHECK:      %TranspTmpArr4 = alloca 64;
; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   |   (%TranspTmpArr4)[i1][i2] = (%b)[i2][i1];
; CHECK:      |   + END LOOP
; CHECK:      + END LOOP
; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   + DO i2 = 0, 3, 1   <DO_LOOP>
;             |   |   %tmp.038 = 0.000000e+00;
;             |   |
; CHECK:      |   |   + DO i3 = 0, 3, 1   <DO_LOOP>
; CHECK:      |   |   |   %mul11 = (%TranspTmpArr4)[i2][i3]  *  (%TranspTmpArr)[i1][i3];
;             |   |   |   %tmp.038 = %mul11  +  %tmp.038;
;             |   |   + END LOOP
;             |   |
;             |   |   %sub = (%result)[sext.i32.i64(%0) * i1 + i2]  -  %tmp.038;
;             |   |   (%result)[sext.i32.i64(%0) * i1 + i2] = %sub;
;             |   + END LOOP
;             + END LOOP
; CHECK:      @llvm.stackrestore(&((%call3)[0]));
; CHECK:      @llvm.stackrestore(&((%call)[0]));
;         END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@stride = dso_local local_unnamed_addr global i32 16, align 4

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @mulsub_block_ORG(float* noalias nocapture noundef %result, float* nocapture noundef readonly %a, float* nocapture noundef readonly %b) local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @stride, align 4, !tbaa !3
  %1 = sext i32 %0 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc20
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc20 ]
  %2 = mul nsw i64 %indvars.iv48, %1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.end
  %indvars.iv44 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next45, %for.end ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %tmp.038 = phi float [ 0.000000e+00, %for.cond4.preheader ], [ %add12, %for.body6 ]
  %3 = mul nsw i64 %indvars.iv, %1
  %4 = add nsw i64 %3, %indvars.iv48
  %arrayidx = getelementptr inbounds float, float* %a, i64 %4
  %5 = load float, float* %arrayidx, align 4, !tbaa !7
  %6 = add nsw i64 %3, %indvars.iv44
  %arrayidx10 = getelementptr inbounds float, float* %b, i64 %6
  %7 = load float, float* %arrayidx10, align 4, !tbaa !7
  %mul11 = fmul fast float %7, %5
  %add12 = fadd fast float %mul11, %tmp.038
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.end, label %for.body6, !llvm.loop !9

for.end:                                          ; preds = %for.body6
  %add12.lcssa = phi float [ %add12, %for.body6 ]
  %8 = add nsw i64 %2, %indvars.iv44
  %arrayidx16 = getelementptr inbounds float, float* %result, i64 %8
  %9 = load float, float* %arrayidx16, align 4, !tbaa !7
  %sub = fsub fast float %9, %add12.lcssa
  store float %sub, float* %arrayidx16, align 4, !tbaa !7
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond47.not = icmp eq i64 %indvars.iv.next45, 4
  br i1 %exitcond47.not, label %for.inc20, label %for.cond4.preheader, !llvm.loop !11

for.inc20:                                        ; preds = %for.end
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond51.not = icmp eq i64 %indvars.iv.next49, 4
  br i1 %exitcond51.not, label %for.end22, label %for.cond1.preheader, !llvm.loop !12

for.end22:                                        ; preds = %for.inc20
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"float", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
