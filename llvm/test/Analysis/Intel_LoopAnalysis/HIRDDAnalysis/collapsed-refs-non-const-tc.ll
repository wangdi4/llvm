
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that (=) (0) ANTI edge is setup for collapsed refs
; for non-constant inner loop.

; Before collapsing:
;<0>          BEGIN REGION { }
;<24>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <nounroll>
;<25>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <nounroll>
;<8>                |   |   %3 = (%A)[zext.i32.i64(%N) * i1 + i2];
;<10>               |   |   (%A)[zext.i32.i64(%N) * i1 + i2] = %3 + 1;
;<25>               |   + END LOOP
;<24>               + END LOOP
;<0>          END REGION

; After collapsing:
;<0>          BEGIN REGION { modified }
;<25>               + DO i1 = 0, (zext.i32.i64(%N) * zext.i32.i64(%N)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <nounroll>
;<8>                |   %3 = (%A)[i1];
;<10>               |   (%A)[i1] = %3 + 1;
;<25>               + END LOOP
;<0>          END REGION


; CHECK: DD graph for function foo:
; CHECK: 8:10 (%A)[zext.i32.i64(%N) * i1 + i2] --> (%A)[zext.i32.i64(%N) * i1 + i2] ANTI (= =) (0 0)

; CHECK: DD graph for function foo:
; CHECK: 8:10 (%A)[i1] --> (%A)[i1] ANTI (=) (0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32* nocapture noundef %A, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i32 %N, 0
  br i1 %cmp19, label %for.cond1.preheader.lr.ph, label %for.end7

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = zext i32 %N to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.inc5, %for.cond1.preheader.lr.ph
  %indvars.iv22 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next23, %for.inc5 ]
  %1 = mul nsw i64 %indvars.iv22, %0
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %2 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %2
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %add4 = add nsw i32 %3, 1
  store i32 %add4, i32* %arrayidx, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0
  br i1 %exitcond.not, label %for.inc5, label %for.body3, !llvm.loop !7

for.inc5:                                         ; preds = %for.body3
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond26.not = icmp eq i64 %indvars.iv.next23, %0
  br i1 %exitcond26.not, label %for.end7.loopexit, label %for.body3.lr.ph, !llvm.loop !10

for.end7.loopexit:                                ; preds = %for.inc5
  br label %for.end7

for.end7:                                         ; preds = %for.end7.loopexit, %entry
  ret void
}

attributes #0 = { argmemonly nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.mustprogress"}
!9 = !{!"llvm.loop.unroll.disable"}
!10 = distinct !{!10, !8, !9}
