; Check narrower SVML calls are correctly lowered to corresponding SVML calls
; or widened if absent in the library.
; RUN: opt -bugpoint-enable-legacy-pm -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sinf4
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x float> @__svml_sinf4_l9(<4 x float> %A)
; CHECK: ret <4 x float> [[RESULT]]

define <4 x float> @test_sinf4(<4 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4(<4 x float> %A)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_idiv4
; CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %a to <4 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %b to <4 x i32>
; CHECK: [[QUOTIENT:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_e9(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <4 x i32> [[QUOTIENT]] to <2 x i64>
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
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> %A, <4 x i32> %B)
; CHECK: ret <4 x float> [[RESULT]]

define <4 x float> @test_sinf4_mask(<4 x float> %A, <4 x i32> %B) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_sinf8
; CHECK: [[RESULT:%.*]] = call fast svml_avx_avx_impl_cc <8 x float> @__svml_sinf8_l9(<8 x float> %A)
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_sinf8(<8 x float> %A) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_sinf8_mask
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <8 x float> @__svml_sinf8_mask_e9(<8 x float> %A, <8 x i32> %B)
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_sinf8_mask(<8 x float> %A, <8 x i32> %B) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_sinf8_mask(<8 x float> %A, <8 x i32> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_idiv8_mask
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %a to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %b to <8 x i32>
; CHECK: [[MASK:%.*]] = bitcast <4 x i64> %c to <8 x i32>
; CHECK: [[QUOTIENT:%.*]] = call svml_avx_cc <8 x i32> @__svml_idiv8_mask_l9(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]], <8 x i32> [[MASK]])
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
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

; CHECK-LABEL: @test_sincosf8
; CHECK: [[RESULT:%.*]] = call svml_avx_avx_impl_cc { <8 x float>, <8 x float> } @__svml_sincosf8_l9(<8 x float> %a)
; CHECK: [[COS:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 1
; CHECK: store <8 x float> [[COS]], ptr %p, align 32
; CHECK: [[SIN:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 0
; CHECK: ret <8 x float> [[SIN]]

define <8 x float> @test_sincosf8(ptr nocapture %p, <8 x float> %a) #1 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8(<8 x float> %a)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, ptr %p, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_sincosf8_mask
; CHECK: [[RESULT:%.*]] = call svml_avx_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask_e9(<8 x float> %B, <8 x i32> %C)
; CHECK: [[COS:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 1
; CHECK: store <8 x float> [[COS]], ptr %A, align 32
; CHECK: [[SIN:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 0
; CHECK: ret <8 x float> [[SIN]]

define <8 x float> @test_sincosf8_mask(ptr nocapture %A, <8 x float> %B, <8 x i32> %C) #1 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float> %B, <8 x i32> %C)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, ptr %A, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_divrem8
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %B to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %C to <8 x i32>
; CHECK: [[RESULT:%.*]] = call svml_avx_avx_impl_cc { <8 x i32>, <8 x i32> } @__svml_idivrem8_l9(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
; CHECK: [[REMAINDER:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 1
; CHECK: [[REMAINDER_PTR_CAST:%.*]] = bitcast ptr %A to ptr
; CHECK: store <8 x i32> [[REMAINDER]], ptr [[REMAINDER_PTR_CAST]], align 32
; CHECK: [[QUOTIENT:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 0
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
; CHECK: ret <4 x i64> [[QUOTIENT_CAST]]

define <4 x i64> @test_divrem8(ptr nocapture %A, <4 x i64> %B, <4 x i64> %C) #1 {
entry:
  %0 = bitcast <4 x i64> %B to <8 x i32>
  %1 = bitcast <4 x i64> %C to <8 x i32>
  %2 = tail call svml_cc { <8 x i32>, <8 x i32> } @__svml_idivrem8(<8 x i32> %0, <8 x i32> %1)
  %3 = extractvalue { <8 x i32>, <8 x i32> } %2, 1
  %4 = bitcast ptr %A to ptr
  store <8 x i32> %3, ptr %4, align 32
  %5 = extractvalue { <8 x i32>, <8 x i32> } %2, 0
  %6 = bitcast <8 x i32> %5 to <4 x i64>
  ret <4 x i64> %6
}

; CHECK-LABEL: @test_cexpf4
; CHECK: [[RESULT:%.*]] = call fast svml_avx_avx_impl_cc <8 x float> @__svml_cexpf4_l9(<8 x float> %A)
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_cexpf4(<8 x float> %A) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_cexpf4(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_cexpf4_mask
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <8 x float> @__svml_cexpf4_mask_e9(<8 x float> %A, <4 x i64> %B)
; CHECK: ret <8 x float> [[RESULT]]

define <8 x float> @test_cexpf4_mask(<8 x float> %A, <4 x i64> %B) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float> %A, <4 x i64> %B)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_sinf2
; CHECK: [[ARG:%.*]] = shufflevector <2 x float> %A, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x float> @__svml_sinf4_l9(<4 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <4 x float> [[RESULT]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: ret <2 x float> [[RESULT_EXTRACT]]

define <2 x float> @test_sinf2(<2 x float> %A) #0 {
entry:
  %0 = tail call fast svml_cc <2 x float> @__svml_sinf2(<2 x float> %A)
  ret <2 x float> %0
}

; CHECK-LABEL: @test_idiv2
; CHECK: [[DIVIDEND:%.*]] = bitcast <1 x i64> %a to <2 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <1 x i64> %b to <2 x i32>
; CHECK: [[DIVIDEND_WIDEN:%.*]] = shufflevector <2 x i32> [[DIVIDEND]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[DIVISOR_WIDEN:%.*]] = shufflevector <2 x i32> [[DIVISOR]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[QUOTIENT:%.*]] = call svml_cc <4 x i32> @__svml_idiv4_e9(<4 x i32> [[DIVIDEND_WIDEN]], <4 x i32> [[DIVISOR_WIDEN]])
; CHECK: [[QUOTIENT_EXTRACT:%.*]] = shufflevector <4 x i32> [[QUOTIENT]], <4 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <2 x i32> [[QUOTIENT_EXTRACT]] to <1 x i64>
; CHECK: ret <1 x i64> [[QUOTIENT_CAST]]

define <1 x i64> @test_idiv2(<1 x i64> %a, <1 x i64> %b) #0 {
entry:
  %0 = bitcast <1 x i64> %a to <2 x i32>
  %1 = bitcast <1 x i64> %b to <2 x i32>
  %2 = tail call svml_cc <2 x i32> @__svml_idiv2(<2 x i32> %0, <2 x i32> %1)
  %3 = bitcast <2 x i32> %2 to <1 x i64>
  ret <1 x i64> %3
}

; CHECK-LABEL: @test_sinf2_mask
; CHECK: [[ARG:%.*]] = shufflevector <2 x float> %A, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[MASK:%.*]] = shufflevector <2 x i32> %B, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> [[ARG]], <4 x i32> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <4 x float> [[RESULT]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: ret <2 x float> [[RESULT_EXTRACT]]

define <2 x float> @test_sinf2_mask(<2 x float> %A, <2 x i32> %B) #0 {
entry:
  %0 = tail call fast svml_cc <2 x float> @__svml_sinf2_mask(<2 x float> %A, <2 x i32> %B)
  ret <2 x float> %0
}

; CHECK-LABEL: @test_sincosf2
; CHECK: [[ARG:%.*]] = shufflevector <2 x float> %a, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4_l9(<4 x float> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <4 x float> [[SIN]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[TMP1:%.*]] = insertvalue { <2 x float>, <2 x float> } undef, <2 x float> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <4 x float> [[COS]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <2 x float>, <2 x float> } [[TMP1]], <2 x float> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <2 x float>, <2 x float> } [[RESULT_EXTRACT]], 1
; CHECK: store <2 x float> [[COS_RET]], ptr %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <2 x float>, <2 x float> } [[RESULT_EXTRACT]], 0
; CHECK: ret <2 x float> [[SIN_RET]]

define <2 x float> @test_sincosf2(ptr nocapture %p, <2 x float> %a) #1 {
entry:
  %0 = tail call svml_cc { <2 x float>, <2 x float> } @__svml_sincosf2(<2 x float> %a)
  %1 = extractvalue { <2 x float>, <2 x float> } %0, 1
  store <2 x float> %1, ptr %p, align 32
  %2 = extractvalue { <2 x float>, <2 x float> } %0, 0
  ret <2 x float> %2
}

; CHECK-LABEL: @test_sincosf2_mask
; CHECK: [[ARG:%.*]] = shufflevector <2 x float> %B, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[MASK:%.*]] = shufflevector <2 x i32> %C, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4_mask_e9(<4 x float> [[ARG]], <4 x i32> [[MASK]])
; CHECK: [[SIN:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <4 x float> [[SIN]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[TMP1:%.*]] = insertvalue { <2 x float>, <2 x float> } undef, <2 x float> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <4 x float> [[COS]], <4 x float> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <2 x float>, <2 x float> } [[TMP1]], <2 x float> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <2 x float>, <2 x float> } [[RESULT_EXTRACT]], 1
; CHECK: store <2 x float> [[COS_RET]], ptr %A, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <2 x float>, <2 x float> } [[RESULT_EXTRACT]], 0
; CHECK: ret <2 x float> [[SIN_RET]]

define <2 x float> @test_sincosf2_mask(ptr nocapture %A, <2 x float> %B, <2 x i32> %C) #1 {
entry:
  %0 = tail call svml_cc { <2 x float>, <2 x float> } @__svml_sincosf2_mask(<2 x float> %B, <2 x i32> %C)
  %1 = extractvalue { <2 x float>, <2 x float> } %0, 1
  store <2 x float> %1, ptr %A, align 32
  %2 = extractvalue { <2 x float>, <2 x float> } %0, 0
  ret <2 x float> %2
}

; CHECK-LABEL: @test_divrem2
; CHECK: [[DIVIDEND:%.*]] = bitcast <1 x i64> %B to <2 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <1 x i64> %C to <2 x i32>
; CHECK: [[DIVIDEND_WIDEN:%.*]] = shufflevector <2 x i32> [[DIVIDEND]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[DIVISOR_WIDEN:%.*]] = shufflevector <2 x i32> [[DIVISOR]], <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call svml_cc { <4 x i32>, <4 x i32> } @__svml_idivrem4_e9(<4 x i32> [[DIVIDEND_WIDEN]], <4 x i32> [[DIVISOR_WIDEN]])
; CHECK: [[QUOTIENT:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 0
; CHECK: [[QUOTIENT_EXTRACT:%.*]] = shufflevector <4 x i32> [[QUOTIENT]], <4 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[TMP1:%.*]] = insertvalue { <2 x i32>, <2 x i32> } undef, <2 x i32> [[QUOTIENT_EXTRACT]], 0
; CHECK: [[REMAINDER:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 1
; CHECK: [[REMAINDER_EXTRACT:%.*]] = shufflevector <4 x i32> [[REMAINDER]], <4 x i32> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <2 x i32>, <2 x i32> } [[TMP1]], <2 x i32> [[REMAINDER_EXTRACT]], 1
; CHECK: [[REMAINDER_RET:%.*]] = extractvalue { <2 x i32>, <2 x i32> } [[RESULT_EXTRACT]], 1
; CHECK: store <2 x i32> [[REMAINDER_RET]], ptr [[REMAINDER_PTR_CAST]], align 32
; CHECK: [[QUOTIENT_RET:%.*]] = extractvalue { <2 x i32>, <2 x i32> } [[RESULT_EXTRACT]], 0
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <2 x i32> [[QUOTIENT_RET]] to <1 x i64>
; CHECK: ret <1 x i64> [[QUOTIENT_CAST]]

define <1 x i64> @test_divrem2(ptr nocapture %A, <1 x i64> %B, <1 x i64> %C) #1 {
entry:
  %0 = bitcast <1 x i64> %B to <2 x i32>
  %1 = bitcast <1 x i64> %C to <2 x i32>
  %2 = tail call svml_cc { <2 x i32>, <2 x i32> } @__svml_idivrem2(<2 x i32> %0, <2 x i32> %1)
  %3 = extractvalue { <2 x i32>, <2 x i32> } %2, 1
  %4 = bitcast ptr %A to ptr
  store <2 x i32> %3, ptr %4, align 32
  %5 = extractvalue { <2 x i32>, <2 x i32> } %2, 0
  %6 = bitcast <2 x i32> %5 to <1 x i64>
  ret <1 x i64> %6
}

declare svml_cc <4 x float> @__svml_sinf4(<4 x float>)

declare svml_cc <4 x i32> @__svml_idiv4(<4 x i32>, <4 x i32>)

declare svml_cc <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)

declare svml_cc <8 x float> @__svml_sinf8(<8 x float>)

declare svml_cc <8 x float> @__svml_sinf8_mask(<8 x float>, <8 x i32>)

declare svml_cc <8 x i32> @__svml_idiv8_mask(<8 x i32>, <8 x i32>, <8 x i32>)

declare svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8(<8 x float>)

declare svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float>, <8 x i32>)

declare svml_cc { <8 x i32>, <8 x i32> } @__svml_idivrem8(<8 x i32>, <8 x i32>)

declare svml_cc <8 x float> @__svml_cexpf4(<8 x float>)

declare svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float>, <4 x i64>)

declare svml_cc <2 x float> @__svml_sinf2(<2 x float>)

declare svml_cc <2 x i32> @__svml_idiv2(<2 x i32>, <2 x i32>)

declare svml_cc <2 x float> @__svml_sinf2_mask(<2 x float>, <2 x i32>)

declare svml_cc { <2 x float>, <2 x float> } @__svml_sincosf2(<2 x float>)

declare svml_cc { <2 x float>, <2 x float> } @__svml_sincosf2_mask(<2 x float>, <2 x i32>)

declare svml_cc { <2 x i32>, <2 x i32> } @__svml_idivrem2(<2 x i32>, <2 x i32>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
