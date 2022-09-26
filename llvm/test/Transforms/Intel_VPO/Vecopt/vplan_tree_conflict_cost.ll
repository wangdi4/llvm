; Checks tree conflict cost for VF=4 and VF=8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -S -enable-new-pm=0 -mattr=+avx512vl,+avx512cd -vplan-cost-model-print-analysis-for-vf=4 -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF4
; RUN: opt -S -mattr=+avx512vl,+avx512cd -vplan-cost-model-print-analysis-for-vf=4 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF4
; RUN: opt -S -enable-new-pm=0 -mattr=+avx512vl,+avx512cd -vplan-cost-model-print-analysis-for-vf=8 -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF8
; RUN: opt -S -mattr=+avx512vl,+avx512cd -vplan-cost-model-print-analysis-for-vf=8 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-VF8

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(float* noalias nocapture noundef %A, i32* nocapture noundef readonly %B, float* noalias nocapture noundef readonly %C) local_unnamed_addr #0 {
;
; float A[8], C[8];
; int B[8]; // conflict idx
; for (int i=0; i<8; ++i) {
;   int index = B[i];
;   A[index] = A[index] + C[i];
; }
;
; CHECK-VF4: Cost 34 for float %vp{{[0-9]+}} = tree-conflict
; CHECK-VF8: Cost 53 for float %vp{{[0-9]+}} = tree-conflict
;
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 8
  %idxprom1 = sext i32 %0 to i64
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds float, float* %C, i64 %indvars.iv
  %2 = load float, float* %arrayidx4, align 4
  %add = fadd fast float %2, %1
  store float %add, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "prefer-vector-width"="512" }
