; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=8 -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s

; HIR
;   BEGIN REGION { modified }
;         %tgu = (%n)/u8;
;         if (0 <u 8 * %tgu)
;         {
;            + DO i1 = 0, 8 * %tgu + -1, 8
;            |   %add.vec = (<8 x float>*)(%B)[i1]  +  (<8 x float>*)(%A)[i1];
;            |   (<8 x float>*)(%C)[i1] = %add.vec;
;            + END LOOP
;         }
;
;         + DO i1 = 8 * %tgu, %n + -1, 1
;         |   %add = (%B)[i1]  +  (%A)[i1];
;         |   (%C)[i1] = %add;
;         + END LOOP
;   END REGION

; Check that the numcachelines calculation takes into account the loop stride of 8.
; numcachlines = Stride (32) * Tripcount (Symbolic = 100/8=12) / 64 = 6
; Since we have 3 refs, total numcachelines is 18. Without stride division,
; it sometimes gets excessively high.

; CHECK: Locality Info for Loop level: 1     NumCacheLines: 18       SpatialCacheLines: 18    TempInvCacheLines: 0     AvgLvalStride: 32        AvgStride: 32
;        Locality Info for Loop level: 1     NumCacheLines: 3        SpatialCacheLines: 3     TempInvCacheLines: 0     AvgLvalStride: 4         AvgStride: 4


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local float @f(i64 %n, float* noalias nocapture readonly %A, float* noalias nocapture readonly %B, float* noalias nocapture %C) local_unnamed_addr #0 {
entry:
  %cmp17 = icmp sgt i64 %n, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %0 = load float, float* %C, align 4, !tbaa !2
  %sub = add nsw i64 %n, -1
  %ptridx7 = getelementptr inbounds float, float* %C, i64 %sub
  %1 = load float, float* %ptridx7, align 4, !tbaa !2
  %add8 = fadd fast float %1, %0
  ret float %add8

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %ptridx = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = load float, float* %ptridx, align 4, !tbaa !2
  %ptridx3 = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %3 = load float, float* %ptridx3, align 4, !tbaa !2
  %add = fadd fast float %3, %2
  %ptridx5 = getelementptr inbounds float, float* %C, i64 %indvars.iv
  store float %add, float* %ptridx5, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !6
}

attributes #0 = { nofree norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
