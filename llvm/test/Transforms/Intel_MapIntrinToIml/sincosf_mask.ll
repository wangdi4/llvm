; Ensure masked sincos is properly translated to high accuracy variant.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: %{{.*}} = call svml_avx512_cc { <16 x float>, <16 x float> } @__svml_sincosf16_ha_mask_z0({ <16 x float>, <16 x float> } %tmp2, <16 x i1> %tmp, <16 x float> %E)
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define <16 x float> @vector_foo(<16 x float>* nocapture %A, <16 x float> %B, <16 x float> %C, i16 zeroext %D, <16 x float> %E) #0 {
entry:
  %tmp = bitcast i16 %D to <16 x i1>
  %tmp1 = insertvalue { <16 x float>, <16 x float> } undef, <16 x float> %B, 0
  %tmp2 = insertvalue { <16 x float>, <16 x float> } %tmp1, <16 x float> %C, 1
  %tmp3 = tail call svml_cc { <16 x float>, <16 x float> } @__svml_sincosf16_mask({ <16 x float>, <16 x float> } %tmp2, <16 x i1> %tmp, <16 x float> %E)
  %tmp4 = extractvalue { <16 x float>, <16 x float> } %tmp3, 1
  store <16 x float> %tmp4, <16 x float>* %A, align 64
  %tmp5 = extractvalue { <16 x float>, <16 x float> } %tmp3, 0
  ret <16 x float> %tmp5
}

declare svml_cc { <16 x float>, <16 x float> } @__svml_sincosf16_mask({ <16 x float>, <16 x float> }, <16 x i1>, <16 x float>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
