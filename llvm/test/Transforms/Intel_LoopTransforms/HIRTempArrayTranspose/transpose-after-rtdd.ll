; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-runtime-dd,hir-temp-array-transpose" -print-after=hir-temp-array-transpose -aa-pipeline="scoped-noalias-aa" -disable-output 2>&1 | FileCheck %s

; Check that Temp Array Transpose successfully transposes this loop that was multiversioned
; by RTDD. We ignore addressof refs of &((%col) and we also ignore the MV Fallblack Loop that
; still has the edge from (%col)[i2 + zext.i32.i64(%0) * i3] -> (%inner)[zext.i32.i64(%0) * i1 + i2]
; on line 38/40. We also treat MV loops differently during checks for unconditional execution.

; HIR Before Transformation

; BEGIN REGION { }
;       %mv.test = &((%inner)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%col)[0]);
;       %mv.test3 = &((%col)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%inner)[0]);
;       %mv.and = %mv.test  &  %mv.test3;
;       %mv.test4 = &((%inner)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%row)[0]);
;       %mv.test5 = &((%row)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%inner)[0]);
;       %mv.and6 = %mv.test4  &  %mv.test5;
;       if (%mv.and == 0 && %mv.and6 == 0)
;       {
;          + DO i1 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 43>
;          |   + DO i2 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 44>
;          |   |   %3 = (%inner)[zext.i32.i64(%0) * i1 + i2];
;          |   |
;          |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 45>
;          |   |   |   %mul15.us = (%col)[i2 + zext.i32.i64(%0) * i3]  *  (%row)[zext.i32.i64(%0) * i1 + i3];
;          |   |   |   %3 = %3  -  %mul15.us;
;          |   |   |   (%inner)[zext.i32.i64(%0) * i1 + i2] = %3;
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;       }
;       else
;       {
;          + DO i1 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 43> <nounroll> <novectorize>
;          |   + DO i2 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 44> <nounroll> <novectorize>
;          |   |   %3 = (%inner)[zext.i32.i64(%0) * i1 + i2];
;          |   |
;          |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>  <MVTag: 45> <nounroll> <novectorize>
;          |   |   |   %mul15.us = (%col)[i2 + zext.i32.i64(%0) * i3]  *  (%row)[zext.i32.i64(%0) * i1 + i3];
;          |   |   |   %3 = %3  -  %mul15.us;
;          |   |   |   (%inner)[zext.i32.i64(%0) * i1 + i2] = %3;
;          |   |   + END LOOP
;          |   + END LOOP
;          + END LOOP
;       }
; END REGION


; HIR After Transformation

; CHECK:    BEGIN REGION { modified }
;                 %mv.test = &((%inner)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%col)[0]);
;                 %mv.test3 = &((%col)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%inner)[0]);
;                 %mv.and = %mv.test  &  %mv.test3;
;                 %mv.test4 = &((%inner)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%row)[0]);
;                 %mv.test5 = &((%row)[(zext.i32.i64(%0) * zext.i32.i64(%0)) + -1]) >=u &((%inner)[0]);
;                 %mv.and6 = %mv.test4  &  %mv.test5;
; CHECK:          %call = @llvm.stacksave();
; CHECK:          %TranspTmpArr = alloca 4 * (zext.i32.i64(%0) * zext.i32.i64(%0));
;
; CHECK:          + DO i1 = 0, zext.i32.i64(%0) + -1, 1
; CHECK:          |   + DO i2 = 0, zext.i32.i64(%0) + -1, 1
; CHECK:          |   |   (%TranspTmpArr)[i1][i2] = (%col)[i2][i1];
;                 |   + END LOOP
;                 + END LOOP

;
; CHECK:          if (%mv.and == 0 && %mv.and6 == 0)
;                 {
;                    + DO i1 = 0, zext.i32.i64(%0) + -1, 1
;                    |   + DO i2 = 0, zext.i32.i64(%0) + -1, 1
;                    |   |   %3 = (%inner)[zext.i32.i64(%0) * i1 + i2];
;                    |   |
;                    |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1
; CHECK:             |   |   |   %mul15.us = (%TranspTmpArr)[i2][i3]  *  (%row)[zext.i32.i64(%0) * i1 + i3];
;                    |   |   |   %3 = %3  -  %mul15.us;
;                    |   |   |   (%inner)[zext.i32.i64(%0) * i1 + i2] = %3;
;                    |   |   + END LOOP
;                    |   + END LOOP
;                    + END LOOP
;                 }
;                 else
;                 {
;                    + DO i1 = 0, zext.i32.i64(%0) + -1, 1
;                    |   + DO i2 = 0, zext.i32.i64(%0) + -1, 1
;                    |   |   %3 = (%inner)[zext.i32.i64(%0) * i1 + i2];
;                    |   |
;                    |   |   + DO i3 = 0, zext.i32.i64(%0) + -1, 1
;                    |   |   |   %mul15.us = (%col)[i2 + zext.i32.i64(%0) * i3]  *  (%row)[zext.i32.i64(%0) * i1 + i3];
;                    |   |   |   %3 = %3  -  %mul15.us;
;                    |   |   |   (%inner)[zext.i32.i64(%0) * i1 + i2] = %3;
;                    |   |   + END LOOP
;                    |   + END LOOP
;                    + END LOOP
;                 }
; CHECK:          @llvm.stackrestore(&((%call)[0]));
; CHECK:    END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@bots_arg_size_1 = external dso_local local_unnamed_addr global i32, align 4
@.str = hidden unnamed_addr constant [65 x i8] c"Checking failure: A[%d][%d]=%f  B[%d][%d]=%f; Relative Error=%f\0A\00", align 1
@.str.1 = hidden unnamed_addr constant [22 x i8] c"Error: Out of memory\0A\00", align 1
@.str.2 = hidden unnamed_addr constant [32 x i8] c"Structure for matrix %s @ 0x%p\0A\00", align 1
@.str.6 = hidden unnamed_addr constant [67 x i8] c"Computing SparseLU Factorization (%dx%d matrix with %dx%d blocks) \00", align 1
@.str.7 = hidden unnamed_addr constant [13 x i8] c" completed!\0A\00", align 1
@.str.8 = hidden unnamed_addr constant [17 x i8] c"Output size: %d\0A\00", align 1
@.str.9 = hidden unnamed_addr constant [34 x i8] c"Output Matrix: A[%d][%d]=%8.12f \0A\00", align 1
@.source.0.0 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.10 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.12 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.14 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.16 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.18 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.20 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.22 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.24 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.26 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.source.0.0.28 = hidden unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @bmod(float* nocapture noundef readonly %row, float* nocapture noundef readonly %col, float* nocapture noundef %inner) local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @bots_arg_size_1, align 4, !tbaa !4
  %cmp45 = icmp sgt i32 %0, 0
  br i1 %cmp45, label %for.cond1.preheader.lr.ph, label %for.end25

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %1 = zext i32 %0 to i64
  br label %for.cond1.preheader.us

for.cond1.preheader.us:                           ; preds = %for.cond1.for.inc23_crit_edge.us, %for.cond1.preheader.lr.ph
  %indvars.iv55 = phi i64 [ %indvars.iv.next56, %for.cond1.for.inc23_crit_edge.us ], [ 0, %for.cond1.preheader.lr.ph ]
  %2 = mul nsw i64 %indvars.iv55, %1
  br label %for.body6.lr.ph.us

for.inc20.us:                                     ; preds = %for.body6.us
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond54.not = icmp eq i64 %indvars.iv.next51, %1
  br i1 %exitcond54.not, label %for.cond1.for.inc23_crit_edge.us, label %for.body6.lr.ph.us, !llvm.loop !8

for.body6.us:                                     ; preds = %for.body6.lr.ph.us, %for.body6.us
  %3 = phi float [ %.pre, %for.body6.lr.ph.us ], [ %sub.us, %for.body6.us ]
  %indvars.iv = phi i64 [ 0, %for.body6.lr.ph.us ], [ %indvars.iv.next, %for.body6.us ]
  %4 = add nuw nsw i64 %indvars.iv, %2
  %arrayidx10.us = getelementptr inbounds float, float* %row, i64 %4
  %5 = load float, float* %arrayidx10.us, align 4, !tbaa !10
  %6 = mul nsw i64 %indvars.iv, %1
  %7 = add nuw nsw i64 %6, %indvars.iv50
  %arrayidx14.us = getelementptr inbounds float, float* %col, i64 %7
  %8 = load float, float* %arrayidx14.us, align 4, !tbaa !10
  %mul15.us = fmul fast float %8, %5
  %sub.us = fsub fast float %3, %mul15.us
  store float %sub.us, float* %arrayidx.us, align 4, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond.not, label %for.inc20.us, label %for.body6.us, !llvm.loop !12

for.body6.lr.ph.us:                               ; preds = %for.inc20.us, %for.cond1.preheader.us
  %indvars.iv50 = phi i64 [ 0, %for.cond1.preheader.us ], [ %indvars.iv.next51, %for.inc20.us ]
  %9 = add nuw nsw i64 %indvars.iv50, %2
  %arrayidx.us = getelementptr inbounds float, float* %inner, i64 %9
  %.pre = load float, float* %arrayidx.us, align 4, !tbaa !10
  br label %for.body6.us

for.cond1.for.inc23_crit_edge.us:                 ; preds = %for.inc20.us
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  %exitcond59.not = icmp eq i64 %indvars.iv.next56, %1
  br i1 %exitcond59.not, label %for.end25.loopexit, label %for.cond1.preheader.us, !llvm.loop !13

for.end25.loopexit:                               ; preds = %for.cond1.for.inc23_crit_edge.us
  br label %for.end25

for.end25:                                        ; preds = %for.end25.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}
!nvvm.annotations = !{}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !6, i64 0}
!12 = distinct !{!12, !9}
!13 = distinct !{!13, !9}
