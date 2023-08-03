;Source code:
;void matrix_mul_matrix(int  N,
;           int *restrict C, int *restrict A, int *restrict B) {
;  int  i,j,k;
;  for (i=0; i<N; i++) {
;    for (j=0; j<N; j++) {
;      C[i*N+j]=0;
;      for(k=0;k<N;k++) {
;        C[i*N+j]+= A[i*N+k] *  B[k*N+j];
;      }
;    }
;  }
;}
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
;<0>          BEGIN REGION { }
;<42>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<43>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<9>                |   |   %3 = 0;
;<44>               |   |
;<44>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<14>               |   |   |   %5 = (%A)[zext.i32.i64(%N) * i1 + i3];
;<18>               |   |   |   %8 = (%B)[i2 + zext.i32.i64(%N) * i3];
;<20>               |   |   |   %3 = %3  +  (%8 * %5);
;<44>               |   |   + END LOOP
;<44>               |   |
;<28>               |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %3;
;<43>               |   + END LOOP
;<42>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;Function: matrix_mul_matrix
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = 0;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   |   |   %3 = (%C)[zext.i32.i64(%N) * i1 + i2];
; CHECK:           |   |   |   %5 = (%A)[zext.i32.i64(%N) * i1 + i3];
; CHECK:           |   |   |   %8 = (%B)[i2 + zext.i32.i64(%N) * i3];
; CHECK:           |   |   |   %3 = %3  +  (%8 * %5);
; CHECK:           |   |   |   (%C)[zext.i32.i64(%N) * i1 + i2] = %3;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't3.c'
source_filename = "t3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @matrix_mul_matrix(i32 %N, ptr noalias nocapture %C, ptr noalias nocapture readonly %A, ptr noalias nocapture readonly %B) local_unnamed_addr #0 {
entry:
  %cmp548 = icmp sgt i32 %N, 0
  br i1 %cmp548, label %for.cond1.preheader.preheader, label %for.end26

for.cond1.preheader.preheader:                    ; preds = %entry
  %0 = zext i32 %N to i64
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.inc24, %for.cond1.preheader.preheader
  %indvars.iv64 = phi i64 [ 0, %for.cond1.preheader.preheader ], [ %indvars.iv.next65, %for.inc24 ]
  %1 = mul nsw i64 %indvars.iv64, %0
  br label %for.body6.lr.ph

for.body6.lr.ph:                                  ; preds = %for.inc21, %for.body3.preheader
  %indvars.iv59 = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next60, %for.inc21 ]
  %2 = add nuw nsw i64 %indvars.iv59, %1
  %arrayidx = getelementptr inbounds i32, ptr %C, i64 %2
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph ], [ %indvars.iv.next, %for.body6 ]
  %3 = phi i32 [ 0, %for.body6.lr.ph ], [ %add20, %for.body6 ]
  %4 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx10 = getelementptr inbounds i32, ptr %A, i64 %4
  %5 = load i32, ptr %arrayidx10, align 4, !tbaa !3
  %6 = mul nsw i64 %indvars.iv, %0
  %7 = add nuw nsw i64 %6, %indvars.iv59
  %arrayidx14 = getelementptr inbounds i32, ptr %B, i64 %7
  %8 = load i32, ptr %arrayidx14, align 4, !tbaa !3
  %mul15 = mul nsw i32 %8, %5
  %add20 = add nsw i32 %3, %mul15
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond.not, label %for.inc21, label %for.body6, !llvm.loop !7

for.inc21:                                        ; preds = %for.body6
  %add20.lcssa = phi i32 [ %add20, %for.body6 ]
  store i32 %add20.lcssa, ptr %arrayidx, align 4, !tbaa !3
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond63.not = icmp eq i64 %indvars.iv.next60, %0
  br i1 %exitcond63.not, label %for.inc24, label %for.body6.lr.ph, !llvm.loop !9

for.inc24:                                        ; preds = %for.inc21
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond68.not = icmp eq i64 %indvars.iv.next65, %0
  br i1 %exitcond68.not, label %for.end26.loopexit, label %for.body3.preheader, !llvm.loop !10

for.end26.loopexit:                               ; preds = %for.inc24
  br label %for.end26

for.end26:                                        ; preds = %for.end26.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
!10 = distinct !{!10, !8}
