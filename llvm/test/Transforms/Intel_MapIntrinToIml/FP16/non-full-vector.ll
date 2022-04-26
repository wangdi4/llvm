; Check narrower FP16 SVML calls are correctly widen to 512-bit SVML calls
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_sins4
; CHECK: [[ARG:%.*]] = shufflevector <4 x half> %A, <4 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <16 x half> @__svml_sins16(<16 x half> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x half> [[RESULT]], <16 x half> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x half> [[RESULT_EXTRACT]]

define <4 x half> @test_sins4(<4 x half> %A) #0 {
entry:
  %0 = tail call fast svml_cc <4 x half> @__svml_sins4(<4 x half> %A)
  ret <4 x half> %0
}

; CHECK-LABEL: @test_sins4_mask
; CHECK: [[ARG:%.*]] = shufflevector <4 x half> %A, <4 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK:%.*]] = shufflevector <4 x i16> %B, <4 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call fast svml_avx_cc <16 x half> @__svml_sins16_mask(<16 x half> [[ARG]], <16 x i16> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <16 x half> [[RESULT]], <16 x half> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x half> [[RESULT_EXTRACT]]

define <4 x half> @test_sins4_mask(<4 x half> %A, <4 x i16> %B) #0 {
entry:
  %0 = tail call fast svml_cc <4 x half> @__svml_sins4_mask(<4 x half> %A, <4 x i16> %B)
  ret <4 x half> %0
}

; CHECK-LABEL: @test_sins16
; CHECK: [[ARG:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <32 x half> @__svml_sins32(<32 x half> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <32 x half> [[RESULT]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: ret <16 x half> [[RESULT_EXTRACT]]

define <16 x half> @test_sins16(<16 x half> %A) #1 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16(<16 x half> %A)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sins16_mask
; CHECK: [[MASK_I1:%.*]] = icmp eq <16 x i16> %B, <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
; CHECK: [[MASK:%.*]] = shufflevector <16 x i1> [[MASK_I1]], <16 x i1> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[ARG:%.*]] = shufflevector <16 x half> %A, <16 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = call fast svml_avx512_cc <32 x half> @__svml_sins32_mask(<32 x half> undef, <32 x i1> [[MASK]], <32 x half> [[ARG]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <32 x half> [[RESULT]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: ret <16 x half> [[RESULT_EXTRACT]]

define <16 x half> @test_sins16_mask(<16 x half> %A, <16 x i16> %B) #1 {
entry:
  %0 = tail call fast svml_cc <16 x half> @__svml_sins16_mask(<16 x half> %A, <16 x i16> %B)
  ret <16 x half> %0
}

; CHECK-LABEL: @test_sincoss16
; CHECK: [[ARG:%.*]] = shufflevector <16 x half> %a, <16 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = call svml_avx512_cc { <32 x half>, <32 x half> } @__svml_sincoss32_ha(<32 x half> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <32 x half> [[SIN]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[TMP1:%.*]] = insertvalue { <16 x half>, <16 x half> } undef, <16 x half> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <32 x half> [[COS]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <16 x half>, <16 x half> } [[TMP1]], <16 x half> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT_EXTRACT]], 1
; CHECK: store <16 x half> [[COS_RET]], <16 x half>* %p, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT_EXTRACT]], 0
; CHECK: ret <16 x half> [[SIN_RET]]

define <16 x half> @test_sincoss16(<16 x half>* nocapture %p, <16 x half> %a) #1 {
entry:
  %0 = tail call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half> %a)
  %1 = extractvalue { <16 x half>, <16 x half> } %0, 1
  store <16 x half> %1, <16 x half>* %p, align 32
  %2 = extractvalue { <16 x half>, <16 x half> } %0, 0
  ret <16 x half> %2
}

; CHECK-LABEL: @test_sincoss16_mask
; CHECK: [[MASK_I1:%.*]] = icmp eq <16 x i16> %C, <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
; CHECK: [[MASK:%.*]] = shufflevector <16 x i1> [[MASK_I1]], <16 x i1> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[ARG:%.*]] = shufflevector <16 x half> %B, <16 x half> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT:%.*]] = call svml_avx512_cc { <32 x half>, <32 x half> } @__svml_sincoss32_ha_mask({ <32 x half>, <32 x half> } undef, <32 x i1> [[MASK]], <32 x half> [[ARG]])
; CHECK: [[SIN:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
; CHECK: [[SIN_EXTRACT:%.*]] = shufflevector <32 x half> [[SIN]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[TMP1:%.*]] = insertvalue { <16 x half>, <16 x half> } undef, <16 x half> [[SIN_EXTRACT]], 0
; CHECK: [[COS:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
; CHECK: [[COS_EXTRACT:%.*]] = shufflevector <32 x half> [[COS]], <32 x half> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT_EXTRACT:%.*]] = insertvalue { <16 x half>, <16 x half> } [[TMP1]], <16 x half> [[COS_EXTRACT]], 1
; CHECK: [[COS_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT_EXTRACT]], 1
; CHECK: store <16 x half> [[COS_RET]], <16 x half>* %A, align 32
; CHECK: [[SIN_RET:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT_EXTRACT]], 0
; CHECK: ret <16 x half> [[SIN_RET]]

define <16 x half> @test_sincoss16_mask(<16 x half>* nocapture %A, <16 x half> %B, <16 x i16> %C) #1 {
entry:
  %0 = tail call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16_mask(<16 x half> %B, <16 x i16> %C)
  %1 = extractvalue { <16 x half>, <16 x half> } %0, 1
  store <16 x half> %1, <16 x half>* %A, align 32
  %2 = extractvalue { <16 x half>, <16 x half> } %0, 0
  ret <16 x half> %2
}

declare svml_cc <4 x half> @__svml_sins4(<4 x half>)

declare svml_cc <4 x half> @__svml_sins4_mask(<4 x half>, <4 x i16>)

declare svml_cc <8 x half> @__svml_sins8(<8 x half>)

declare svml_cc <16 x half> @__svml_sins16(<16 x half>)

declare svml_cc <16 x half> @__svml_sins16_mask(<16 x half>, <16 x i16>)

declare svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half>)

declare svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16_mask(<16 x half>, <16 x i16>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="512" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+avx512fp16,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
