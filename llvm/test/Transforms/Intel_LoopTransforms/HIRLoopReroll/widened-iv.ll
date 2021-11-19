; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-pre-vec-complete-unroll -hir-loop-reroll -print-after=hir-loop-reroll < %s 2>& 1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange,hir-pre-vec-complete-unroll,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>& 1 | FileCheck %s

; Verifiy that compiler doesn't die. Previously, it was crashing at HIRVerifier
; because of missing live-in of the i5 loop's UB.

; *** IR Dump After HIR Loop Reroll (hir-loop-reroll) ***
; Function: func2

;          BEGIN REGION { modified }
;                + DO i1 = 0, 4, 1   <DO_LOOP>
;                |   + DO i2 = 0, 1, 1   <DO_LOOP>
;                |   |   %2 = (-1 * i2 + -1 != 0) ? 4 : 8;
;                |   |
;                |   |   + DO i3 = 0, 3, 1   <DO_LOOP>
;                |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
; CHECK:         |   |   |   |      %sext = sext.i32.i64(-64 * i2 + 16 * %2 + -1);
; CHECK:         |   |   |   |   + DO i5 = 0, %sext, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;                |   |   |   |   |   (%A)[20 * i1 + 5 * i3 + i4][64 * i2 + i5] = 20000 * i1 + 64 * i2 + 5000 * i3 + 1000 * i4 + i5;
;                |   |   |   |   + END LOOP
;                |   |   |   + END LOOP
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP
;          END REGION

; ModuleID = 'module.ll'
source_filename = "omp51_tile2.c"
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable writeonly
define dso_local void @func2([128 x i32]* nocapture %A) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %.floor_0.iv.i1.089 = phi i32 [ 0, %entry ], [ %0, %for.cond.cleanup3 ]
  %0 = add nuw nsw i32 %.floor_0.iv.i1.089, 4
  br label %for.body10.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.body10.preheader:                             ; preds = %for.cond.cleanup9, %for.cond1.preheader
  %1 = phi i1 [ false, %for.cond.cleanup9 ], [ true, %for.cond1.preheader ]
  %.floor_1.iv.j1.087 = phi i32 [ 4, %for.cond.cleanup9 ], [ 0, %for.cond1.preheader ]
  %2 = select i1 %1, i32 4, i32 8
  %cmp2083 = icmp ult i32 %.floor_1.iv.j1.087, %2
  br label %for.body10

for.cond.cleanup3:                                ; preds = %for.cond.cleanup9
  %cmp = icmp ult i32 %.floor_0.iv.i1.089, 16
  br i1 %cmp, label %for.cond1.preheader, label %for.cond.cleanup, !llvm.loop !5

for.cond.cleanup9:                                ; preds = %for.cond.cleanup21
  br i1 %1, label %for.body10.preheader, label %for.cond.cleanup3, !llvm.loop !7

for.body10:                                       ; preds = %for.cond.cleanup21, %for.body10.preheader
  %.tile_0.iv.i1.086 = phi i32 [ %inc45, %for.cond.cleanup21 ], [ %.floor_0.iv.i1.089, %for.body10.preheader ]
  %mul = mul nuw nsw i32 %.tile_0.iv.i1.086, 5
  %add26 = add nuw nsw i32 %mul, 5
  br i1 %cmp2083, label %for.body22.preheader, label %for.cond.cleanup21

for.body22.preheader:                             ; preds = %for.body10
  br label %for.body22

for.cond.cleanup21.loopexit:                      ; preds = %for.cond.cleanup28
  br label %for.cond.cleanup21

for.cond.cleanup21:                               ; preds = %for.cond.cleanup21.loopexit, %for.body10
  %inc45 = add nuw nsw i32 %.tile_0.iv.i1.086, 1
  %exitcond92.not = icmp eq i32 %inc45, %0
  br i1 %exitcond92.not, label %for.cond.cleanup9, label %for.body10, !llvm.loop !8

for.body22:                                       ; preds = %for.cond.cleanup28, %for.body22.preheader
  %.tile_1.iv.j1.084 = phi i32 [ %inc42, %for.cond.cleanup28 ], [ %.floor_1.iv.j1.087, %for.body22.preheader ]
  %mul23 = shl nsw i32 %.tile_1.iv.j1.084, 4
  %add31 = add nuw nsw i32 %mul23, 16
  br label %for.cond30.preheader

for.cond30.preheader:                             ; preds = %for.cond.cleanup33, %for.body22
  %i2.082 = phi i32 [ %mul, %for.body22 ], [ %inc39, %for.cond.cleanup33 ]
  %mul35 = mul nuw nsw i32 %i2.082, 1000
  br label %for.body34

for.cond.cleanup28:                               ; preds = %for.cond.cleanup33
  %inc42 = add nuw nsw i32 %.tile_1.iv.j1.084, 1
  %exitcond91.not = icmp eq i32 %inc42, %2
  br i1 %exitcond91.not, label %for.cond.cleanup21.loopexit, label %for.body22, !llvm.loop !9

for.cond.cleanup33:                               ; preds = %for.body34
  %inc39 = add nuw nsw i32 %i2.082, 1
  %exitcond90.not = icmp eq i32 %inc39, %add26
  br i1 %exitcond90.not, label %for.cond.cleanup28, label %for.cond30.preheader, !llvm.loop !10

for.body34:                                       ; preds = %for.body34, %for.cond30.preheader
  %j2.081 = phi i32 [ %mul23, %for.cond30.preheader ], [ %inc, %for.body34 ]
  %add36 = add nuw nsw i32 %j2.081, %mul35
  %arrayidx37 = getelementptr inbounds [128 x i32], [128 x i32]* %A, i32 %i2.082, i32 %j2.081
  store i32 %add36, i32* %arrayidx37, align 4, !tbaa !11
  %inc = add nuw nsw i32 %j2.081, 1
  %exitcond.not = icmp eq i32 %inc, %add31
  br i1 %exitcond.not, label %for.cond.cleanup33, label %for.body34, !llvm.loop !16
}

attributes #0 = { nofree norecurse nosync nounwind uwtable writeonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7" "target-features"="+crc32,+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}
!nvvm.annotations = !{}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !6}
!10 = distinct !{!10, !6}
!11 = !{!12, !13, i64 0}
!12 = !{!"array@_ZTSA128_i", !13, i64 0}
!13 = !{!"int", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = distinct !{!16, !6}
