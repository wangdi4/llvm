; for (i = 0;  i < N;   i++) {
;
;   at1 = 4 * a1[i] / 3;
;   at2 = 4 * a2[i] / 3;
;   atyp = atype[i] - 1;
;
;   rx = x[at1] - x[at2];
;   ry = x[at1 + 1] - x[at2 + 1];
;   rz = x[at1 + 2] - x[at2 + 2];
;   r2 = rx * rx + ry * ry + rz * rz;
;
;   rw = x[at1 + 3] - x[at2 + 3];
;   r2 += rw * rw;
;
;   s = sqrt(r2);
;   r = 2.0 / s;
;   db = s - Req[atyp];
;   df = Rk[atyp] * db;
;   e = df * db;
;   e_bond += e;
;   df *= r;
;
;   f[foff + at1 + 0] += rx * df;
;   f[foff + at1 + 1] += ry * df;
;   f[foff + at1 + 2] += rz * df;
;   f[foff + at2 + 0] -= rx * df;
;   f[foff + at2 + 1] -= ry * df;
;   f[foff + at2 + 2] -= rz * df;
;   f[foff + at1 + 3] += rw * df;
;   f[foff + at2 + 3] -= rw * df;
; }

; RUN: opt < %s -loop-simplify -hir-ssa-deconstruction | opt -analyze -hir-cost-model-throttling=0 -force-hir-sparse-array-reduction-analysis -hir-sparse-array-reduction-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="loop-simplify,hir-ssa-deconstruction,print<hir-sparse-array-reduction-analysis>" -hir-cost-model-throttling=0 -force-hir-sparse-array-reduction-analysis -disable-output 2>&1 | FileCheck %s
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>
; CHECK:   <Sparse Array Reduction>

;Module Before HIR; ModuleID = 'nab.c'
source_filename = "nab.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e_bond = common local_unnamed_addr global float 0.000000e+00, align 4
@a1 = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@a2 = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@atype = common local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@x = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Req = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Rk = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@f = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @nab(i32 %N, i64 %foff) local_unnamed_addr #0 {
entry:
  store float 0.000000e+00, float* @e_bond, align 4, !tbaa !2
  %conv = sext i32 %N to i64
  %cmp146 = icmp sgt i32 %N, 0
  br i1 %cmp146, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.0147 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x i64], [1000 x i64]* @a1, i64 0, i64 %i.0147
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %mul = shl nsw i64 %0, 2
  %div = sdiv i64 %mul, 3
  %arrayidx2 = getelementptr inbounds [1000 x i64], [1000 x i64]* @a2, i64 0, i64 %i.0147
  %1 = load i64, i64* %arrayidx2, align 8, !tbaa !6
  %mul3 = shl nsw i64 %1, 2
  %div4 = sdiv i64 %mul3, 3
  %arrayidx5 = getelementptr inbounds [1000 x i64], [1000 x i64]* @atype, i64 0, i64 %i.0147
  %2 = load i64, i64* %arrayidx5, align 8, !tbaa !6
  %sub = add nsw i64 %2, -1
  %arrayidx6 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %div
  %3 = load float, float* %arrayidx6, align 4, !tbaa !9
  %arrayidx7 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %div4
  %4 = load float, float* %arrayidx7, align 4, !tbaa !9
  %sub8 = fsub float %3, %4
  %add = add nsw i64 %div, 1
  %arrayidx9 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add
  %5 = load float, float* %arrayidx9, align 4, !tbaa !9
  %add10 = add nsw i64 %div4, 1
  %arrayidx11 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add10
  %6 = load float, float* %arrayidx11, align 4, !tbaa !9
  %sub12 = fsub float %5, %6
  %add13 = add nsw i64 %div, 2
  %arrayidx14 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add13
  %7 = load float, float* %arrayidx14, align 4, !tbaa !9
  %add15 = add nsw i64 %div4, 2
  %arrayidx16 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add15
  %8 = load float, float* %arrayidx16, align 4, !tbaa !9
  %sub17 = fsub float %7, %8
  %mul18 = fmul float %sub8, %sub8
  %mul19 = fmul float %sub12, %sub12
  %add20 = fadd float %mul18, %mul19
  %mul21 = fmul float %sub17, %sub17
  %add22 = fadd float %add20, %mul21
  %add23 = add nsw i64 %div, 3
  %arrayidx24 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add23
  %9 = load float, float* %arrayidx24, align 4, !tbaa !9
  %add25 = add nsw i64 %div4, 3
  %arrayidx26 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add25
  %10 = load float, float* %arrayidx26, align 4, !tbaa !9
  %sub27 = fsub float %9, %10
  %mul28 = fmul float %sub27, %sub27
  %add29 = fadd float %add22, %mul28
  %sqrtf = tail call float @sqrtf(float %add29) #1
  %conv34 = fdiv float 2.000000e+00, %sqrtf
  %arrayidx35 = getelementptr inbounds [1000 x float], [1000 x float]* @Req, i64 0, i64 %sub
  %11 = load float, float* %arrayidx35, align 4, !tbaa !9
  %sub36 = fsub float %sqrtf, %11
  %arrayidx37 = getelementptr inbounds [1000 x float], [1000 x float]* @Rk, i64 0, i64 %sub
  %12 = load float, float* %arrayidx37, align 4, !tbaa !9
  %mul38 = fmul float %12, %sub36
  %mul39 = fmul float %sub36, %mul38
  %13 = load float, float* @e_bond, align 4, !tbaa !2
  %add40 = fadd float %13, %mul39
  store float %add40, float* @e_bond, align 4, !tbaa !2
  %mul41 = fmul float %conv34, %mul38
  %mul42 = fmul float %sub8, %mul41
  %add43 = add nsw i64 %div, %foff
  %arrayidx45 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add43
  %14 = load float, float* %arrayidx45, align 4, !tbaa !9
  %add46 = fadd float %14, %mul42
  store float %add46, float* %arrayidx45, align 4, !tbaa !9
  %mul47 = fmul float %sub12, %mul41
  %add49 = add nsw i64 %add43, 1
  %arrayidx50 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add49
  %15 = load float, float* %arrayidx50, align 4, !tbaa !9
  %add51 = fadd float %mul47, %15
  store float %add51, float* %arrayidx50, align 4, !tbaa !9
  %mul52 = fmul float %sub17, %mul41
  %add54 = add nsw i64 %add43, 2
  %arrayidx55 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add54
  %16 = load float, float* %arrayidx55, align 4, !tbaa !9
  %add56 = fadd float %mul52, %16
  store float %add56, float* %arrayidx55, align 4, !tbaa !9
  %add58 = add nsw i64 %div4, %foff
  %arrayidx60 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add58
  %17 = load float, float* %arrayidx60, align 4, !tbaa !9
  %sub61 = fsub float %17, %mul42
  store float %sub61, float* %arrayidx60, align 4, !tbaa !9
  %add64 = add nsw i64 %add58, 1
  %arrayidx65 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add64
  %18 = load float, float* %arrayidx65, align 4, !tbaa !9
  %sub66 = fsub float %18, %mul47
  store float %sub66, float* %arrayidx65, align 4, !tbaa !9
  %add69 = add nsw i64 %add58, 2
  %arrayidx70 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add69
  %19 = load float, float* %arrayidx70, align 4, !tbaa !9
  %sub71 = fsub float %19, %mul52
  store float %sub71, float* %arrayidx70, align 4, !tbaa !9
  %mul72 = fmul float %sub27, %mul41
  %add74 = add nsw i64 %add43, 3
  %arrayidx75 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add74
  %20 = load float, float* %arrayidx75, align 4, !tbaa !9
  %add76 = fadd float %mul72, %20
  store float %add76, float* %arrayidx75, align 4, !tbaa !9
  %add79 = add nsw i64 %add58, 3
  %arrayidx80 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add79
  %21 = load float, float* %arrayidx80, align 4, !tbaa !9
  %sub81 = fsub float %21, %mul72
  store float %sub81, float* %arrayidx80, align 4, !tbaa !9
  %inc = add nuw nsw i64 %i.0147, 1
  %exitcond = icmp eq i64 %inc, %conv
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 0
}

declare float @sqrtf(float) local_unnamed_addr

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 508c5f8755c4ef14fa1fbfdc76c7e07d843afe24)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA1000_l", !8, i64 0}
!8 = !{!"long", !4, i64 0}
!9 = !{!10, !3, i64 0}
!10 = !{!"array@_ZTSA1000_f", !3, i64 0}
