
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that DD preserves (<) backward dependency after collapsing
; and sets correct distance (a product of corresponding TCs).

; Before collapsing
;<0>          BEGIN REGION { }
;<36>               + DO i1 = 0, 9, 1   <DO_LOOP> <nounroll>
;<37>               |   + DO i2 = 0, 9, 1   <DO_LOOP> <nounroll>
;<38>               |   |   + DO i3 = 0, 9, 1   <DO_LOOP> <nounroll>
;<13>               |   |   |   %add = (@A)[0][i1][i2][i3]  +  1.000000e+00;
;<14>               |   |   |   %add17 = %add  +  (@B)[0][i1][i2][i3];
;<16>               |   |   |   (@A)[0][i1 + 1][i2][i3] = %add17;
;<38>               |   |   + END LOOP
;<37>               |   + END LOOP
;<36>               + END LOOP
;<0>          END REGION

; After collapsing
;<0>          BEGIN REGION { modified }
;<38>               + DO i1 = 0, 999, 1   <DO_LOOP> <nounroll>
;<13>               |   %add = (@A)[0][0][0][i1]  +  1.000000e+00;
;<14>               |   %add17 = %add  +  (@B)[0][0][0][i1];
;<16>               |   (@A)[0][1][0][i1] = %add17;
;<38>               + END LOOP
;<0>          END REGION

; CHECK: DD graph for function foo:
; CHECK: 16:13 (@A)[0][i1 + 1][i2][i3] --> (@A)[0][i1][i2][i3] FLOW (< = =) (1 0 0)

; CHECK: DD graph for function foo:
; CHECK: 16:13 (@A)[0][1][0][i1] --> (@A)[0][0][0][i1] FLOW (<) (100)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [10 x [10 x [10 x float]]] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [10 x [10 x [10 x float]]] zeroinitializer, align 16
@C = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  %C.promoted = load i32, i32* @C, align 4, !tbaa !3
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc29
  %indvars.iv62 = phi i64 [ 0, %entry ], [ %indvars.iv.next63, %for.inc29 ]
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond1.preheader, %for.inc26
  %indvars.iv59 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next60, %for.inc26 ]
  br label %for.body6

for.body6:                                        ; preds = %for.cond4.preheader, %for.body6
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx10 = getelementptr inbounds [10 x [10 x [10 x float]]], [10 x [10 x [10 x float]]]* @A, i64 0, i64 %indvars.iv62, i64 %indvars.iv59, i64 %indvars.iv, !intel-tbaa !7
  %0 = load float, float* %arrayidx10, align 4, !tbaa !7
  %arrayidx16 = getelementptr inbounds [10 x [10 x [10 x float]]], [10 x [10 x [10 x float]]]* @B, i64 0, i64 %indvars.iv62, i64 %indvars.iv59, i64 %indvars.iv, !intel-tbaa !7
  %1 = load float, float* %arrayidx16, align 4, !tbaa !7
  %add = fadd fast float %0, 1.000000e+00
  %add17 = fadd fast float %add, %1
  %arrayidx24 = getelementptr inbounds [10 x [10 x [10 x float]]], [10 x [10 x [10 x float]]]* @A, i64 0, i64 %indvars.iv.next63, i64 %indvars.iv59, i64 %indvars.iv, !intel-tbaa !7
  store float %add17, float* %arrayidx24, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond.not, label %for.inc26, label %for.body6, !llvm.loop !12

for.inc26:                                        ; preds = %for.body6
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond61.not = icmp eq i64 %indvars.iv.next60, 10
  br i1 %exitcond61.not, label %for.inc29, label %for.cond4.preheader, !llvm.loop !15

for.inc29:                                        ; preds = %for.inc26
  %exitcond64.not = icmp eq i64 %indvars.iv.next63, 10
  br i1 %exitcond64.not, label %for.end31, label %for.cond1.preheader, !llvm.loop !16

for.end31:                                        ; preds = %for.inc29
  %2 = add i32 %C.promoted, 1000
  store i32 %2, i32* @C, align 4, !tbaa !3
  %3 = load float, float* getelementptr inbounds ([10 x [10 x [10 x float]]], [10 x [10 x [10 x float]]]* @A, i64 0, i64 0, i64 0, i64 0), align 16, !tbaa !7
  %4 = load float, float* getelementptr inbounds ([10 x [10 x [10 x float]]], [10 x [10 x [10 x float]]]* @B, i64 0, i64 1, i64 1, i64 1), align 4, !tbaa !7
  %add32 = fadd fast float %3, 1.000000e+00
  %add33 = fadd fast float %add32, %4
  %conv = fptosi float %add33 to i32
  ret i32 %conv
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !11, i64 0}
!8 = !{!"array@_ZTSA10_A10_A10_f", !9, i64 0}
!9 = !{!"array@_ZTSA10_A10_f", !10, i64 0}
!10 = !{!"array@_ZTSA10_f", !11, i64 0}
!11 = !{!"float", !5, i64 0}
!12 = distinct !{!12, !13, !14}
!13 = !{!"llvm.loop.mustprogress"}
!14 = !{!"llvm.loop.unroll.disable"}
!15 = distinct !{!15, !13, !14}
!16 = distinct !{!16, !13, !14}
