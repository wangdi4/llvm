;

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-distribute-loopnest,hir-loop-interchange,print<hir>" -aa-pipeline="tbaa" 2>&1 < %s | FileCheck %s 

;  MATMUL for SI16 - best form is in DOTPRODUCT 
;  for (i=0; i<N; i++) {
;    for (j=0; j<N; j++) {
;      C[i*N+j]=0;
;      for(k=0;k<N;k++) {;
;	C[i*N+j]+=(MATRES)A[i*N+k] * (MATRES)B[k*N+j];
;      }
;    }
;  }
;
; HIR after TempCleanup
;
;      + DO i1 = 0, %N + -1, 1   <DO_LOOP>  
;      |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP> 
;      |   |   %1 = 0;
;      |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  
;      |   |   |   %3 = (%A)[%N * i1 + i3];
;      |   |   |   %4 = (%B)[i2 + %N * i3];
;      |   |   |   %1 = (sext.i16.i32(%3) * sext.i16.i32(%4))  +  %1;
;      |   |   + END LOOP
;      |   |   (%C)[%N * i1 + i2] = %1;
;      |   + END LOOP
;      + END LOOP
;
; HIR after Interchange
;
;
; CHECK:    BEGIN REGION { }
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:       |   |   (%C)[%N * i1 + i2] = 0;
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:       |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:       |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:       |   |   |   %1 = (%C)[%N * i1 + i2];
; CHECK:       |   |   |   %3 = (%A)[%N * i1 + i3];
; CHECK:       |   |   |   %4 = (%B)[i2 + %N * i3];
; CHECK:       |   |   |   %1 = (sext.i16.i32(%3) * sext.i16.i32(%4))  +  %1;
; CHECK:       |   |   |   (%C)[%N * i1 + i2] = %1;
; CHECK:       |   |   + END LOOP
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK:    END REGION

;Module Before HIR
; ModuleID = 'int16MATMUL.c'
source_filename = "int16MATMUL.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @matrix_mul_matrix(i32 noundef %N, ptr nocapture noundef writeonly %C, ptr nocapture noundef readonly %A, ptr nocapture noundef readonly %B) local_unnamed_addr #0 {
entry:
  %cmp48.not = icmp eq i32 %N, 0
  br i1 %cmp48.not, label %for.end27, label %for.cond1.preheader.preheader

for.cond1.preheader.preheader:                    ; preds = %entry
  %wide.trip.count52 = zext i32 %N to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.preheader, %for.inc25
  %i.049 = phi i32 [ %inc26, %for.inc25 ], [ 0, %for.cond1.preheader.preheader ]
  %mul = mul i32 %i.049, %N
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc22
  %indvars.iv50 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next51, %for.inc22 ]
  %0 = trunc i64 %indvars.iv50 to i32
  %add = add i32 %mul, %0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i32, ptr %C, i64 %idxprom
  br label %for.body6

for.body6:                                        ; preds = %for.body3, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body3 ], [ %indvars.iv.next, %for.body6 ]
  %1 = phi i32 [ 0, %for.body3 ], [ %add21, %for.body6 ]
  %2 = trunc i64 %indvars.iv to i32
  %add8 = add i32 %mul, %2
  %idxprom9 = zext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i16, ptr %A, i64 %idxprom9
  %3 = load i16, ptr %arrayidx10, align 2, !tbaa !3
  %conv = sext i16 %3 to i32
  %mul11 = mul i32 %2, %N
  %add12 = add i32 %mul11, %0
  %idxprom13 = zext i32 %add12 to i64
  %arrayidx14 = getelementptr inbounds i16, ptr %B, i64 %idxprom13
  %4 = load i16, ptr %arrayidx14, align 2, !tbaa !3
  %conv15 = sext i16 %4 to i32
  %mul16 = mul nsw i32 %conv15, %conv
  %add21 = add nsw i32 %mul16, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count52
  br i1 %exitcond.not, label %for.inc22, label %for.body6, !llvm.loop !7

for.inc22:                                        ; preds = %for.body6
  %add21.lcssa = phi i32 [ %add21, %for.body6 ]
  store i32 %add21.lcssa, ptr %arrayidx, align 4, !tbaa !9
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond53.not = icmp eq i64 %indvars.iv.next51, %wide.trip.count52
  br i1 %exitcond53.not, label %for.inc25, label %for.body3, !llvm.loop !11

for.inc25:                                        ; preds = %for.inc22
  %inc26 = add nuw i32 %i.049, 1
  %exitcond54.not = icmp eq i32 %inc26, %N
  br i1 %exitcond54.not, label %for.end27.loopexit, label %for.cond1.preheader, !llvm.loop !12

for.end27.loopexit:                               ; preds = %for.inc25
  br label %for.end27

for.end27:                                        ; preds = %for.end27.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.0.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"short", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !5, i64 0}
!11 = distinct !{!11, !8}
!12 = distinct !{!12, !8}
