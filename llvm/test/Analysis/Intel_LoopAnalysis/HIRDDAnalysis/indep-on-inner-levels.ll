; RUN: opt -passes="require<scoped-noalias-aa>,hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; Test check that DV for %0[] and %1[] dd refs is formed as (* 0 0) and
; DD analysis does not assert on NONE DV in the middle of the vector.

; C source:
;
; void foo(float ** restrict Ap, float** restrict Bp, int* restrict c, int n) {
;   for(int i=0; i<1024; i++) {
;       float* restrict A = Ap[i];
;       float* restrict B = Bp[i];
;       for(int j=0; j<1024; j++){
;#pragma ivdep
;         for(int k=0; k<1024; k++) {
;           A[j*n + c[k]] += B[k];
;         }
;       }
;     }
; }

;     BEGIN REGION { }
;        + DO i1 = 0, 1023, 1   <DO_LOOP>
;        |   %0 = (%Ap)[i1];
;        |   %1 = (%Bp)[i1];
;        |
;        |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;        |   |   + DO i3 = 0, 1023, 1   <DO_LOOP> <ivdep>
;        |   |   |   %3 = (%c)[i3];
;        |   |   |   %add17 = (%0)[sext.i32.i64(%n) * i2 + sext.i32.i64(%3)]  +  (%1)[i3];
;        |   |   |   (%0)[sext.i32.i64(%n) * i2 + sext.i32.i64(%3)] = %add17;
;        |   |   + END LOOP
;        |   + END LOOP
;        + END LOOP
;     END REGION

; CHECK: DD graph for function foo:
; CHECK: (%0)[sext.i32.i64(%n) * i2 + sext.i32.i64(%3)] --> (%1)[i3] FLOW (* 0 0) (? ? ?)
; CHECK-NOT: (%0)[sext.i32.i64(%n) * i2 + sext.i32.i64(%3)] --> (%1)[i3] {{.*}} (* 0 =) (? ? 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(ptr noalias nocapture noundef readonly %Ap, ptr noalias nocapture noundef readonly %Bp, ptr noalias nocapture noundef readonly %c, i32 noundef %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv41 = phi i64 [ 0, %entry ], [ %indvars.iv.next42, %for.cond.cleanup5 ]
  %arrayidx = getelementptr inbounds ptr, ptr %Ap, i64 %indvars.iv41
  %0 = load ptr, ptr %arrayidx, align 8, !tbaa !3
  %arrayidx2 = getelementptr inbounds ptr, ptr %Bp, i64 %indvars.iv41
  %1 = load ptr, ptr %arrayidx2, align 8, !tbaa !3
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.body, %for.cond.cleanup9
  %j.038 = phi i32 [ 0, %for.body ], [ %inc19, %for.cond.cleanup9 ]
  %mul = mul nsw i32 %j.038, %n
  br label %for.body10

for.cond.cleanup5:                                ; preds = %for.cond.cleanup9
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43.not = icmp eq i64 %indvars.iv.next42, 1024
  br i1 %exitcond43.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7

for.cond.cleanup9:                                ; preds = %for.body10
  %inc19 = add nuw nsw i32 %j.038, 1
  %exitcond40.not = icmp eq i32 %inc19, 1024
  br i1 %exitcond40.not, label %for.cond.cleanup5, label %for.cond7.preheader, !llvm.loop !9

for.body10:                                       ; preds = %for.cond7.preheader, %for.body10
  %indvars.iv = phi i64 [ 0, %for.cond7.preheader ], [ %indvars.iv.next, %for.body10 ]
  %arrayidx12 = getelementptr inbounds float, ptr %1, i64 %indvars.iv
  %2 = load float, ptr %arrayidx12, align 4, !tbaa !10, !alias.scope !12, !noalias !15
  %arrayidx14 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx14, align 4, !tbaa !17
  %add = add nsw i32 %3, %mul
  %idxprom15 = sext i32 %add to i64
  %arrayidx16 = getelementptr inbounds float, ptr %0, i64 %idxprom15
  %4 = load float, ptr %arrayidx16, align 4, !tbaa !10, !alias.scope !15, !noalias !12
  %add17 = fadd fast float %4, %2
  store float %add17, ptr %arrayidx16, align 4, !tbaa !10, !alias.scope !15, !noalias !12
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.cond.cleanup9, label %for.body10, !llvm.loop !19
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPf", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !8}
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !5, i64 0}
!12 = !{!13}
!13 = distinct !{!13, !14, !"foo: B"}
!14 = distinct !{!14, !"foo"}
!15 = !{!16}
!16 = distinct !{!16, !14, !"foo: A"}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !5, i64 0}
!19 = distinct !{!19, !8, !20}
!20 = !{!"llvm.loop.vectorize.ivdep_back"}
