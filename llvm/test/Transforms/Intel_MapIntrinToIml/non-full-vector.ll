; Check narrower SVML calls are correctly widen to 512-bit SVML calls
; RUN: opt -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sinf4
; CHECK: [[ARG:%.*]] = shufflevector <4 x float> %A, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call svml_cc <8 x float> @__svml_sinf8(<8 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <8 x float> [[RESULT]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x float> [[RESULT_EXTRACT]]

define <4 x float> @test_sinf4(<4 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4(<4 x float> %A)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_idiv4
; CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %a to <4 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %b to <4 x i32>
; CHECK: [[DIVIDEND_WIDEN:%.*]] = shufflevector <4 x i32> [[DIVIDEND]], <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[DIVISOR_WIDEN:%.*]] = shufflevector <4 x i32> [[DIVISOR]], <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT:%.*]] = call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> [[DIVIDEND_WIDEN]], <8 x i32> [[DIVISOR_WIDEN]])
; CHECK: [[QUOTIENT_EXTRACT:%.*]] = shufflevector <8 x i32> [[QUOTIENT]], <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <4 x i32> [[QUOTIENT_EXTRACT]] to <2 x i64>
; CHECK: ret <2 x i64> [[QUOTIENT_CAST]]

define <2 x i64> @test_idiv4(<2 x i64> %a, <2 x i64> %b) #0 {
entry:
  %0 = bitcast <2 x i64> %a to <4 x i32>
  %1 = bitcast <2 x i64> %b to <4 x i32>
  %2 = tail call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> %0, <4 x i32> %1)
  %3 = bitcast <4 x i32> %2 to <2 x i64>
  ret <2 x i64> %3
}

; CHECK-LABEL: @test_sinf4_mask
; CHECK: [[ARG:%.*]] = shufflevector <4 x float> %A, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK:%.*]] = shufflevector <4 x i32> %B, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call svml_cc <8 x float> @__svml_sinf8_mask(<8 x float> [[ARG]], <8 x i32> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <8 x float> [[RESULT]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x float> [[RESULT_EXTRACT]]

define <4 x float> @test_sinf4_mask(<4 x float> %A, <4 x i32> %B) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_sinf8
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call svml_cc <16 x float> @__svml_sinf16(<16 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x float> [[RESULT]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT_EXTRACT]]

define <8 x float> @test_sinf8(<8 x float> %A) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_sinf8_mask
; CHECK: [[MASK_I1:%.*]] = icmp eq <8 x i32> %B, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
; CHECK: [[MASK:%.*]] = shufflevector <8 x i1> [[MASK_I1]], <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> undef, <16 x i1> [[MASK]], <16 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x float> [[RESULT]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT_EXTRACT]]

define <8 x float> @test_sinf8_mask(<8 x float> %A, <8 x i32> %B) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8_mask(<8 x float> %A, <8 x i32> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_idiv8_mask
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %a to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %b to <8 x i32>
; CHECK: [[MASK:%.*]] = bitcast <4 x i64> %c to <8 x i32>
; CHECK: [[MASK_I1:%.*]] = icmp eq <8 x i32> [[MASK]], <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
; CHECK: [[MASK_WIDEN:%.*]] = shufflevector <8 x i1> [[MASK_I1]], <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVIDEND_WIDEN:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR_WIDEN:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT:%.*]] = call svml_cc <16 x i32> @__svml_idiv16_mask(<16 x i32> undef, <16 x i1> [[MASK_WIDEN]], <16 x i32> [[DIVIDEND_WIDEN]], <16 x i32> [[DIVISOR_WIDEN]])
; CHECK: [[QUOTIENT_EXTRACT:%.*]] = shufflevector <16 x i32> [[QUOTIENT]], <16 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT_EXTRACT]] to <4 x i64>
; CHECK: ret <4 x i64> [[QUOTIENT_CAST]]

define <4 x i64> @test_idiv8_mask(<4 x i64> %a, <4 x i64> %b, <4 x i64> %c) #1 {
entry:
  %0 = bitcast <4 x i64> %a to <8 x i32>
  %1 = bitcast <4 x i64> %b to <8 x i32>
  %2 = bitcast <4 x i64> %c to <8 x i32>
  %3 = tail call svml_cc <8 x i32> @__svml_idiv8_mask(<8 x i32> %0, <8 x i32> %1, <8 x i32> %2)
  %4 = bitcast <8 x i32> %3 to <4 x i64>
  ret <4 x i64> %4
}

declare svml_cc <4 x float> @__svml_sinf4(<4 x float>)

declare svml_cc <4 x i32> @__svml_idiv4(<4 x i32>, <4 x i32>)

declare svml_cc <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)

declare svml_cc <8 x float> @__svml_sinf8(<8 x float>)

declare svml_cc <8 x float> @__svml_sinf8_mask(<8 x float>, <8 x i32>)

declare svml_cc <8 x i32> @__svml_idiv8_mask(<8 x i32>, <8 x i32>, <8 x i32>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
