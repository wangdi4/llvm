; Check narrower FP16 SVML calls are correctly lowered to corresponding SVML
; calls or widened if absent in the library.
; RUN: opt -bugpoint-enable-legacy-pm -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sins4
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x half> @__svml_sins4(<4 x half> %A)
; CHECK: ret <4 x half> [[RESULT]]

define <4 x half> @test_sins4(<4 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x half> @__svml_sins4(<4 x half> %A)
  ret <4 x half> %0
}

; CHECK-LABEL: @test_sins4_mask
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x half> @__svml_sins4_mask(<4 x half> %A, <4 x i16> %B)
; CHECK: ret <4 x half> [[RESULT]]

define <4 x half> @test_sins4_mask(<4 x half> %A, <4 x i16> %B) #0 {
entry:
  %0 = tail call fast svml_cc <4 x half> @__svml_sins4_mask(<4 x half> %A, <4 x i16> %B)
  ret <4 x half> %0
}

; CHECK-LABEL: @test_sins16
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <16 x half> @__svml_sins16(<16 x half> [[ARG:%.*]])
; CHECK: ret <16 x half> [[RESULT]]

define <16 x half> @test_sins16(<16 x half> %A) #1 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16(<16 x half> %A)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sins16_mask
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <16 x half> @__svml_sins16_mask(<16 x half> [[ARG:%.*]], <16 x i16> [[MASK:%.*]])
; CHECK: ret <16 x half> [[RESULT]]

define <16 x half> @test_sins16_mask(<16 x half> %A, <16 x i16> %B) #1 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16_mask(<16 x half> %A, <16 x i16> %B)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sincoss16
; CHECK: [[RESULT:%.*]] = call svml_avx_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half> [[ARG:%.*]])
; CHECK: [[COS_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 1
; CHECK: store <16 x half> [[COS_RET]], ptr %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 0
; CHECK: ret <16 x half> [[SIN_RET]]

define <16 x half> @test_sincoss16(ptr nocapture %p, <16 x half> %a) #1 {
entry:
  %0 = tail call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half> %a)
  %1 = extractvalue { <16 x half>, <16 x half> } %0, 1
  store <16 x half> %1, ptr %p, align 32
  %2 = extractvalue { <16 x half>, <16 x half> } %0, 0
  ret <16 x half> %2
}

; CHECK-LABEL: @test_sincoss16_mask
; CHECK: [[RESULT:%.*]] = call svml_avx_cc { <16 x half>, <16 x half> } @__svml_sincoss16_mask(<16 x half> [[ARG:%.*]], <16 x i16> [[MASK:%.*]])
; CHECK: [[COS_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 1
; CHECK: store <16 x half> [[COS_RET]], ptr %A, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 0
; CHECK: ret <16 x half> [[SIN_RET]]

define <16 x half> @test_sincoss16_mask(ptr nocapture %A, <16 x half> %B, <16 x i16> %C) #1 {
entry:
  %0 = tail call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16_mask(<16 x half> %B, <16 x i16> %C)
  %1 = extractvalue { <16 x half>, <16 x half> } %0, 1
  store <16 x half> %1, ptr %A, align 32
  %2 = extractvalue { <16 x half>, <16 x half> } %0, 0
  ret <16 x half> %2
}

; CHECK-LABEL: @test_sins2
; CHECK: [[ARG:%.*]] = shufflevector <2 x half> %A, <2 x half> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x half> @__svml_sins4(<4 x half> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <4 x half> [[RESULT]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: ret <2 x half> [[RESULT_EXTRACT]]

define <2 x half> @test_sins2(<2 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <2 x half> @__svml_sins2(<2 x half> %A)
  ret <2 x half> %0
}

; CHECK-LABEL: @test_sins2_mask
; CHECK: [[ARG:%.*]] = shufflevector <2 x half> %A, <2 x half> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[MASK:%.*]] = shufflevector <2 x i16> %B, <2 x i16> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call fast svml_cc <4 x half> @__svml_sins4_mask(<4 x half> [[ARG]], <4 x i16> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <4 x half> [[RESULT]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: ret <2 x half> [[RESULT_EXTRACT]]

define <2 x half> @test_sins2_mask(<2 x half> %A, <2 x i16> %B) #0 {
entry:
  %0 = tail call fast svml_cc <2 x half> @__svml_sins2_mask(<2 x half> %A, <2 x i16> %B)
  ret <2 x half> %0
}

; CHECK-LABEL: @test_sincoss2
; CHECK: [[ARG:%.*]] = shufflevector <2 x half> %a, <2 x half> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call svml_cc { <4 x half>, <4 x half> } @__svml_sincoss4(<4 x half> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <4 x half>, <4 x half> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <4 x half> [[SIN]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[TMP1:%.*]] = insertvalue { <2 x half>, <2 x half> } undef, <2 x half> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <4 x half>, <4 x half> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <4 x half> [[COS]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <2 x half>, <2 x half> } [[TMP1]], <2 x half> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <2 x half>, <2 x half> } [[RESULT_EXTRACT]], 1
; CHECK: store <2 x half> [[COS_RET]], ptr %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <2 x half>, <2 x half> } [[RESULT_EXTRACT]], 0
; CHECK: ret <2 x half> [[SIN_RET]]

define <2 x half> @test_sincoss2(ptr nocapture %p, <2 x half> %a) #1 {
entry:
  %0 = tail call svml_cc { <2 x half>, <2 x half> } @__svml_sincoss2(<2 x half> %a)
  %1 = extractvalue { <2 x half>, <2 x half> } %0, 1
  store <2 x half> %1, ptr %p, align 32
  %2 = extractvalue { <2 x half>, <2 x half> } %0, 0
  ret <2 x half> %2
}

; CHECK-LABEL: @test_sincoss2_mask
; CHECK: [[ARG:%.*]] = shufflevector <2 x half> %B, <2 x half> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[MASK:%.*]] = shufflevector <2 x i16> %C, <2 x i16> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK: [[RESULT:%.*]] = call svml_cc { <4 x half>, <4 x half> } @__svml_sincoss4_mask(<4 x half> [[ARG]], <4 x i16> [[MASK]])
; CHECK: [[SIN:%.*]] = extractvalue { <4 x half>, <4 x half> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <4 x half> [[SIN]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[TMP1:%.*]] = insertvalue { <2 x half>, <2 x half> } undef, <2 x half> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <4 x half>, <4 x half> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <4 x half> [[COS]], <4 x half> undef, <2 x i32> <i32 0, i32 1>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <2 x half>, <2 x half> } [[TMP1]], <2 x half> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <2 x half>, <2 x half> } [[RESULT_EXTRACT]], 1
; CHECK: store <2 x half> [[COS_RET]], ptr %A, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <2 x half>, <2 x half> } [[RESULT_EXTRACT]], 0
; CHECK: ret <2 x half> [[SIN_RET]]

define <2 x half> @test_sincoss2_mask(ptr nocapture %A, <2 x half> %B, <2 x i16> %C) #1 {
entry:
  %0 = tail call svml_cc { <2 x half>, <2 x half> } @__svml_sincoss2_mask(<2 x half> %B, <2 x i16> %C)
  %1 = extractvalue { <2 x half>, <2 x half> } %0, 1
  store <2 x half> %1, ptr %A, align 32
  %2 = extractvalue { <2 x half>, <2 x half> } %0, 0
  ret <2 x half> %2
}

declare svml_cc <4 x half> @__svml_sins4(<4 x half>)

declare svml_cc <4 x half> @__svml_sins4_mask(<4 x half>, <4 x i16>)

declare svml_cc <8 x half> @__svml_sins8(<8 x half>)

declare svml_cc <16 x half> @__svml_sins16(<16 x half>)

declare svml_cc <16 x half> @__svml_sins16_mask(<16 x half>, <16 x i16>)

declare svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half>)

declare svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16_mask(<16 x half>, <16 x i16>)

declare svml_cc <2 x half> @__svml_sins2(<2 x half>)

declare svml_cc <2 x half> @__svml_sins2_mask(<2 x half>, <2 x i16>)

declare svml_cc { <2 x half>, <2 x half> } @__svml_sincoss2(<2 x half>)

declare svml_cc { <2 x half>, <2 x half> } @__svml_sincoss2_mask(<2 x half>, <2 x i16>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
