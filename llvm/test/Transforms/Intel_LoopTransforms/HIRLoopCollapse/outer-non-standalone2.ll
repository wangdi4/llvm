; RUN: opt -debug-only=hir-loop-collapse -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

;[Note]
; (%A)[i1 + i2][i2] is invalid for collapsing on dim2 because high-level iv (e.g. i2) is not allowed on high dimensions.


;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 499, 1   <DO_LOOP>
; CHECK:           |   |   (%A)[i1 + i2][i2] = 0;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK:     Dimension number 2 is illegal to collapse

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 499, 1   <DO_LOOP>
; CHECK:           |   |   (%A)[i1 + i2][i2] = 0;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo(ptr nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc6
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv22
  %arrayidx5 = getelementptr inbounds [500 x i32], ptr %A, i64 %0, i64 %indvars.iv
  store i32 0, ptr %arrayidx5, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 500
  br i1 %exitcond.not, label %for.inc6, label %for.body3, !llvm.loop !8

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24.not = icmp eq i64 %indvars.iv.next23, 500
  br i1 %exitcond24.not, label %for.end8, label %for.cond1.preheader, !llvm.loop !10

for.end8:                                         ; preds = %for.inc6
  %arrayidx10 = getelementptr inbounds [500 x i32], ptr %A, i64 1, i64 7
  %1 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  ret i32 %1
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA500_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
