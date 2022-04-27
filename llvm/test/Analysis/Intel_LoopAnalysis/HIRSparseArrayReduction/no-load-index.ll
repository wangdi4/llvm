; for (i = 0;  i < N;   i++) {
;   at1 = 4 * i / 3;
;   f[foff + at1] += i;
; }

; RUN: opt < %s -analyze -enable-new-pm=0 -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="loop-simplify,hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s
; CHECK:   No Sparse Array Reduction

;Module Before HIR; ModuleID = 'no-load-index.c'
source_filename = "no-load-index.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e_bond = common local_unnamed_addr global float 0.000000e+00, align 4
@f = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@a1 = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@a2 = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@atype = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@x = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Req = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Rk = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @nab(i32 %N, i64 %foff) local_unnamed_addr #0 {
entry:
  store float 0.000000e+00, float* @e_bond, align 4, !tbaa !2
  %conv = sext i32 %N to i64
  %cmp21 = icmp sgt i32 %N, 0
  br i1 %cmp21, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.022 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul = shl nsw i64 %i.022, 2
  %div = udiv i64 %mul, 3
  %conv2 = sitofp i64 %i.022 to float
  %add = add nsw i64 %div, %foff
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add
  %0 = load float, float* %arrayidx, align 4, !tbaa !6
  %add3 = fadd float %0, %conv2
  store float %add3, float* %arrayidx, align 4, !tbaa !6
  %inc = add nuw nsw i64 %i.022, 1
  %exitcond = icmp eq i64 %inc, %conv
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm b3533268f84ee0fd5034deed68765542f2a54304)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_f", !3, i64 0}
