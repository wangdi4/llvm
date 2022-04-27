; Check SVML calls absent in the library falls back to scalarized calls with "f16" suffix.
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @test_frexp
; CHECK: [[ARG1:%.*]] = extractelement <4 x half> %A, i32 0
; CHECK: [[RESULT1:%.*]] = call half @frexpf16(half [[ARG1]])
; CHECK: [[ARG2:%.*]] = extractelement <4 x half> %A, i32 1
; CHECK: [[RESULT2:%.*]] = call half @frexpf16(half [[ARG2]])
; CHECK: [[ARG3:%.*]] = extractelement <4 x half> %A, i32 2
; CHECK: [[RESULT3:%.*]] = call half @frexpf16(half [[ARG3]])
; CHECK: [[ARG4:%.*]] = extractelement <4 x half> %A, i32 3
; CHECK: [[RESULT4:%.*]] = call half @frexpf16(half [[ARG4]])
; CHECK: [[INS1:%.*]] = insertelement <4 x half> undef, half [[RESULT1]], i32 0
; CHECK: [[INS2:%.*]] = insertelement <4 x half> [[INS1]], half [[RESULT2]], i32 1
; CHECK: [[INS3:%.*]] = insertelement <4 x half> [[INS2]], half [[RESULT3]], i32 2
; CHECK: [[INS4:%.*]] = insertelement <4 x half> [[INS3]], half [[RESULT4]], i32 3
; CHECK: ret <4 x half> [[INS4]]

define <4 x half> @test_frexp(<4 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x half> @__svml_frexps4(<4 x half> %A)
  ret <4 x half> %0
}

declare <4 x half> @__svml_frexps4(<4 x half>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
