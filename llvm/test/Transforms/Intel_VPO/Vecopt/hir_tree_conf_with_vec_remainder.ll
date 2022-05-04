; RUN: opt %s -vplan-vec-scenario="n0;v8;v4" -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-non-masked-vectorized-remainder -print-after=hir-vplan-vec -disable-output

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test vec scenario where main loop VF=8, remainder loop VF=4 with tree conflict idiom
; Tree conflict lowering should not be done just before code gen because VF for
; cloned remainder VPlan is not available. This leads to an invalid cloned permute
; intrinsic because type is <4 x i32> and that intrinsic does not exist. We must use
; the float (ps) version.

; CHECK: llvm.x86.avx.vpermilvar.ps

; Function Attrs: mustprogress uwtable
define dso_local void @_ZN9LAMMPS_NS9Irregular11create_atomEiPiS1_i(i32* noalias nocapture noundef readonly %sizes, i32* noalias nocapture noundef readonly %proclist, i64 %N) local_unnamed_addr #0 align 2 {
entry:
  br label %for.body43

for.body43:                                       ; preds = %for.body43, %entry
  %indvars.iv457 = phi i64 [ 0, %entry ], [ %indvars.iv.next458, %for.body43 ]
  %arrayidx45 = getelementptr inbounds i32, i32* %sizes, i64 %indvars.iv457
  %0 = load i32, i32* %arrayidx45, align 4
  %arrayidx48 = getelementptr inbounds i32, i32* %proclist, i64 %indvars.iv457
  %1 = load i32, i32* %arrayidx48, align 4
  %idxprom49 = sext i32 %1 to i64
  %arrayidx50 = getelementptr inbounds i32, i32* null, i64 %idxprom49
  %2 = load i32, i32* %arrayidx50, align 4
  %add = add nsw i32 %2, %0
  store i32 %add, i32* %arrayidx50, align 4
  %indvars.iv.next458 = add nuw nsw i64 %indvars.iv457, 1
  %exitcond460.not = icmp eq i64 %indvars.iv.next458, %N
  br i1 %exitcond460.not, label %for.end53.loopexit, label %for.body43

for.end53.loopexit:                               ; preds = %for.body43
  ret void
}

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!nvvm.annotations = !{}
