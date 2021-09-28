; RUN: opt --O2 %s -S 2>&1 | FileCheck %s
; This case checks that "atanf" has "readonly" attribute under "fast-math" so
; that the "atanf" call can be eliminated by Early-CSE after const folding.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define void @mm_mp_foo_
; CHECK-NOT: atanf
; Function Attrs: mustprogress nofree nounwind uwtable willreturn
define void @mm_mp_foo_(float* noalias nocapture readonly dereferenceable(4) %A, float* noalias nocapture readonly dereferenceable(4) %B, float* noalias nocapture dereferenceable(4) %C) local_unnamed_addr #1 {
alloca_1:
  %func_result = tail call fast float @atanf(float 1.000000e+00)
  %A_fetch.2 = load float, float* %A, align 1
  %mul.2 = fmul fast float %A_fetch.2, 0x400921FB60000000
  %B_fetch.3 = load float, float* %B, align 1
  %div.1 = fdiv fast float %mul.2, %B_fetch.3
  store float %div.1, float* %C, align 1
  ret void
}

; Function Attrs: mustprogress nofree nounwind willreturn
declare float @atanf(float) local_unnamed_addr #2

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nounwind uwtable willreturn "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nofree nounwind willreturn }

!omp_offload.info = !{}
