;for (int i = 0; i < N; i++) {
;      A[B[i]] += c;
;}

; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="loop-simplify,hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s
; CHECK:   %1 = (@A)[0][%0]; <Sparse Array Reduction>
; CHECK:   %add = %1  +  %conv; <Sparse Array Reduction>
; CHECK:   (@A)[0][%0] = %add; <Sparse Array Reduction>

;Module Before HIR; ModuleID = 'sum.c'
source_filename = "sum.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sum(i32 %N, i32 %c) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp sgt i32 %N, 0
  br i1 %cmp5, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %conv = sitofp i32 %c to float
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i64], [1000 x i64]* @B, i64 0, i64 %indvars.iv
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !2
  %arrayidx1 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %0
  %1 = load float, float* %arrayidx1, align 4, !tbaa !7
  %add = fadd float %1, %conv
  store float %add, float* %arrayidx1, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm ac60afebd81ca964f6aee4eb29d0bce587442884)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_l", !4, i64 0}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA1000_f", !9, i64 0}
!9 = !{!"float", !5, i64 0}
