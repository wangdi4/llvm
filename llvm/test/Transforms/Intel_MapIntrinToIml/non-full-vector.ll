; Check narrower SVML calls are correctly widen to 512-bit SVML calls
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sinf4
; CHECK: [[ARG:%.*]] = shufflevector <4 x float> %A, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <8 x float> @__svml_sinf8_l9(<8 x float> [[ARG]])
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
; CHECK: [[QUOTIENT:%.*]] = call svml_avx_cc <8 x i32> @__svml_idiv8_l9(<8 x i32> [[DIVIDEND_WIDEN]], <8 x i32> [[DIVISOR_WIDEN]])
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
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <8 x float> @__svml_sinf8_mask_e9(<8 x float> [[ARG]], <8 x i32> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <8 x float> [[RESULT]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x float> [[RESULT_EXTRACT]]

define <4 x float> @test_sinf4_mask(<4 x float> %A, <4 x i32> %B) #0 {
entry:
  %0 = tail call fast svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_sinf8
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <16 x float> @__svml_sinf16_z0(<16 x float> [[ARG]])
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
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <16 x float> @__svml_sinf16_mask_z0(<16 x float> undef, <16 x i1> [[MASK]], <16 x float> [[ARG]])
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
; CHECK: [[QUOTIENT:%.*]] = call svml_avx512_cc <16 x i32> @__svml_idiv16_mask_z0(<16 x i32> undef, <16 x i1> [[MASK_WIDEN]], <16 x i32> [[DIVIDEND_WIDEN]], <16 x i32> [[DIVISOR_WIDEN]])
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

; CHECK-LABEL: @test_sincosf8
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %a, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call svml_avx512_cc { <16 x float>, <16 x float> } @__svml_sincosf16_ha_z0(<16 x float> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <16 x float> [[SIN]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[TMP1:%.*]] = insertvalue { <8 x float>, <8 x float> } undef, <8 x float> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <16 x float> [[COS]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <8 x float>, <8 x float> } [[TMP1]], <8 x float> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT_EXTRACT]], 1
; CHECK: store <8 x float> [[COS_RET]], <8 x float>* %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT_EXTRACT]], 0
; CHECK: ret <8 x float> [[SIN_RET]]

define <8 x float> @test_sincosf8(<8 x float>* nocapture %p, <8 x float> %a) #1 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8(<8 x float> %a)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, <8 x float>* %p, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_sincosf8_mask
; CHECK: [[MASK_I1:%.*]] = icmp eq <8 x i32> %C, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
; CHECK: [[MASK:%.*]] = shufflevector <8 x i1> [[MASK_I1]], <8 x i1> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %B, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call svml_avx512_cc { <16 x float>, <16 x float> } @__svml_sincosf16_ha_mask_z0({ <16 x float>, <16 x float> } undef, <16 x i1> [[MASK]], <16 x float> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <16 x float> [[SIN]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[TMP1:%.*]] = insertvalue { <8 x float>, <8 x float> } undef, <8 x float> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <16 x float> [[COS]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <8 x float>, <8 x float> } [[TMP1]], <8 x float> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT_EXTRACT]], 1
; CHECK: store <8 x float> [[COS_RET]], <8 x float>* %A, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT_EXTRACT]], 0
; CHECK: ret <8 x float> [[SIN_RET]]

define <8 x float> @test_sincosf8_mask(<8 x float>* nocapture %A, <8 x float> %B, <8 x i32> %C) #1 {
entry:
  %0 = tail call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8_mask(<8 x float> %B, <8 x i32> %C)
  %1 = extractvalue { <8 x float>, <8 x float> } %0, 1
  store <8 x float> %1, <8 x float>* %A, align 32
  %2 = extractvalue { <8 x float>, <8 x float> } %0, 0
  ret <8 x float> %2
}

; CHECK-LABEL: @test_divrem8
; CHECK: [[DIVIDEND:%.*]] = bitcast <4 x i64> %B to <8 x i32>
; CHECK: [[DIVISOR:%.*]] = bitcast <4 x i64> %C to <8 x i32>
; CHECK: [[DIVIDEND_WIDEN:%.*]] = shufflevector <8 x i32> [[DIVIDEND]], <8 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[DIVISOR_WIDEN:%.*]] = shufflevector <8 x i32> [[DIVISOR]], <8 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call svml_avx512_cc { <16 x i32>, <16 x i32> } @__svml_idivrem16_z0(<16 x i32> [[DIVIDEND_WIDEN]], <16 x i32> [[DIVISOR_WIDEN]])
; CHECK: [[QUOTIENT:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[RESULT]], 0
; CHECK: [[QUOTIENT_EXTRACT:%.*]] = shufflevector <16 x i32> [[QUOTIENT]], <16 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[TMP1:%.*]] = insertvalue { <8 x i32>, <8 x i32> } undef, <8 x i32> [[QUOTIENT_EXTRACT]], 0
; CHECK: [[REMAINDER:%.*]] = extractvalue { <16 x i32>, <16 x i32> } [[RESULT]], 1
; CHECK: [[REMAINDER_EXTRACT:%.*]] = shufflevector <16 x i32> [[REMAINDER]], <16 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <8 x i32>, <8 x i32> } [[TMP1]], <8 x i32> [[REMAINDER_EXTRACT]], 1
; CHECK: [[REMAINDER_RET:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT_EXTRACT]], 1
; CHECK: [[REMAINDER_PTR_CAST:%.*]] = bitcast <4 x i64>* %A to <8 x i32>*
; CHECK: store <8 x i32> [[REMAINDER_RET]], <8 x i32>* [[REMAINDER_PTR_CAST]], align 32
; CHECK: [[QUOTIENT_RET:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT_EXTRACT]], 0
; CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT_RET]] to <4 x i64>
; CHECK: ret <4 x i64> [[QUOTIENT_CAST]]

define <4 x i64> @test_divrem8(<4 x i64>* nocapture %A, <4 x i64> %B, <4 x i64> %C) #1 {
entry:
  %0 = bitcast <4 x i64> %B to <8 x i32>
  %1 = bitcast <4 x i64> %C to <8 x i32>
  %2 = tail call svml_cc { <8 x i32>, <8 x i32> } @__svml_idivrem8(<8 x i32> %0, <8 x i32> %1)
  %3 = extractvalue { <8 x i32>, <8 x i32> } %2, 1
  %4 = bitcast <4 x i64>* %A to <8 x i32>*
  store <8 x i32> %3, <8 x i32>* %4, align 32
  %5 = extractvalue { <8 x i32>, <8 x i32> } %2, 0
  %6 = bitcast <8 x i32> %5 to <4 x i64>
  ret <4 x i64> %6
}

; CHECK-LABEL: @test_cexpf4
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <16 x float> @__svml_cexpf8_z0(<16 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x float> [[RESULT]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT_EXTRACT]]

define <8 x float> @test_cexpf4(<8 x float> %A) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_cexpf4(<8 x float> %A)
  ret <8 x float> %0
}

; CHECK-LABEL: @test_cexpf4_mask
; CHECK: [[MASK_I1:%.*]] = icmp eq <4 x i64> %B, <i64 -1, i64 -1, i64 -1, i64 -1>
; CHECK: [[MASK:%.*]] = shufflevector <4 x i1> [[MASK_I1]], <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[ARG:%.*]] = shufflevector <8 x float> %A, <8 x float> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <16 x float> @__svml_cexpf8_mask_z0(<16 x float> undef, <8 x i1> [[MASK]], <16 x float> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x float> [[RESULT]], <16 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: ret <8 x float> [[RESULT_EXTRACT]]

define <8 x float> @test_cexpf4_mask(<8 x float> %A, <4 x i64> %B) #1 {
entry:
  %0 = tail call fast svml_cc <8 x float> @__svml_cexpf4_mask(<8 x float> %A, <4 x i64> %B)
  ret <8 x float> %0
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

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
