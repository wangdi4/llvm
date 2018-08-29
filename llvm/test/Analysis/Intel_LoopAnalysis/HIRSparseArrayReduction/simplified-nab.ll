; for (i = 0;  i < N;   i++) {
;   at1 = 4 * a1[i] / 3;
;   f[foff + at1] += i;
; }

; HIR-
; + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; |   %0 = (@a1)[0][i1];
; |   %div = 4 * %0  /  3;
; |   %conv2 = sitofp.i64.float(i1);
; |   %1 = (@f)[0][%foff + %div];
; |   %add4 = %1  +  %conv2;
; |   (@f)[0][%foff + %div] = %add4;
; + END LOOP

; RUN: opt < %s -analyze -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="loop-simplify,hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s
; CHECK:   %1 = (@f)[0][%foff + %div]; <Sparse Array Reduction>
; CHECK:   %add4 = %1  +  %conv2; <Sparse Array Reduction>
; CHECK:   (@f)[0][%foff + %div] = %add4; <Sparse Array Reduction>

;Module Before HIR; ModuleID = 'simplified-nab.c'
source_filename = "simplified-nab.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e_bond = common local_unnamed_addr global float 0.000000e+00, align 4
@a1 = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@f = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
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
  %cmp22 = icmp sgt i32 %N, 0
  br i1 %cmp22, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.023 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i64], [1000 x i64]* @a1, i64 0, i64 %i.023
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %mul = shl nsw i64 %0, 2
  %div = sdiv i64 %mul, 3
  %conv2 = sitofp i64 %i.023 to float
  %add = add nsw i64 %div, %foff
  %arrayidx3 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add
  %1 = load float, float* %arrayidx3, align 4, !tbaa !9
  %add4 = fadd float %1, %conv2
  store float %add4, float* %arrayidx3, align 4, !tbaa !9
  %inc = add nuw nsw i64 %i.023, 1
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
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA1000_l", !8, i64 0}
!8 = !{!"long", !4, i64 0}
!9 = !{!10, !3, i64 0}
!10 = !{!"array@_ZTSA1000_f", !3, i64 0}
