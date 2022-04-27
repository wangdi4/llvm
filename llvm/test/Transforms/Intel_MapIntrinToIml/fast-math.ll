; Check that SVML call transformation preserves fast math flags.
; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @test_fmf
; CHECK: [[RESULT:%.*]] = call nnan afn svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> %A, <4 x i32> %B)
; CHECK: ret <4 x float> [[RESULT]]

define <4 x float> @test_fmf(<4 x float> %A, <4 x i32> %B) #0 {
entry:
  %0 = tail call nnan afn svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_nofmf
; CHECK: [[RESULT:%.*]] = call svml_cc <4 x float> @__svml_sinf4_ha_mask_e9(<4 x float> %A, <4 x i32> %B)
; CHECK: ret <4 x float> [[RESULT]]

define <4 x float> @test_nofmf(<4 x float> %A, <4 x i32> %B) #0 {
entry:
  %0 = tail call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_split_fmf
; CHECK: [[MASK:%.*]] = bitcast i16 %B to <16 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <16 x i1> [[MASK]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>, <16 x i32> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call nnan afn svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> [[ARG1]], <4 x i32> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call nnan afn svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> [[ARG2]], <4 x i32> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[MASK3:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call nnan afn svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> [[ARG3]], <4 x i32> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK4:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call nnan afn svml_cc <4 x float> @__svml_sinf4_mask_e9(<4 x float> [[ARG4]], <4 x i32> [[MASK4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT_COMBINED:%.*]] = select nnan afn <16 x i1> [[MASK]], <16 x float> [[RESULT]], <16 x float> %A
; CHECK: ret <16 x float> [[RESULT_COMBINED]]

define <16 x float> @test_split_fmf(<16 x float> %A, i16 zeroext %B, <16 x float> %C) #0 {
entry:
  %0 = bitcast i16 %B to <16 x i1>
  %1 = tail call nnan afn svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> %A, <16 x i1> %0, <16 x float> %C)
  ret <16 x float> %1
}

; CHECK-LABEL: @test_split_nofmf
; CHECK: [[MASK:%.*]] = bitcast i16 %B to <16 x i1>
; CHECK: [[MASK_CAST:%.*]] = select <16 x i1> [[MASK]], <16 x i32> <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>, <16 x i32> zeroinitializer
; CHECK: [[ARG1:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK1:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT1:%.*]] = call svml_cc <4 x float> @__svml_sinf4_ha_mask_e9(<4 x float> [[ARG1]], <4 x i32> [[MASK1]])
; CHECK: [[ARG2:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[MASK2:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT2:%.*]] = call svml_cc <4 x float> @__svml_sinf4_ha_mask_e9(<4 x float> [[ARG2]], <4 x i32> [[MASK2]])
; CHECK: [[ARG3:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[MASK3:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; CHECK: [[RESULT3:%.*]] = call svml_cc <4 x float> @__svml_sinf4_ha_mask_e9(<4 x float> [[ARG3]], <4 x i32> [[MASK3]])
; CHECK: [[ARG4:%.*]] = shufflevector <16 x float> %C, <16 x float> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[MASK4:%.*]] = shufflevector <16 x i32> [[MASK_CAST]], <16 x i32> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT4:%.*]] = call svml_cc <4 x float> @__svml_sinf4_ha_mask_e9(<4 x float> [[ARG4]], <4 x i32> [[MASK4]])
; CHECK: [[RESULT12:%.*]] = shufflevector <4 x float> [[RESULT1]], <4 x float> [[RESULT2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT34:%.*]] = shufflevector <4 x float> [[RESULT3]], <4 x float> [[RESULT4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK: [[RESULT:%.*]] = shufflevector <8 x float> [[RESULT12]], <8 x float> [[RESULT34]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK: [[RESULT_COMBINED:%.*]] = select <16 x i1> [[MASK]], <16 x float> [[RESULT]], <16 x float> %A
; CHECK: ret <16 x float> [[RESULT_COMBINED]]

define <16 x float> @test_split_nofmf(<16 x float> %A, i16 zeroext %B, <16 x float> %C) #0 {
entry:
  %0 = bitcast i16 %B to <16 x i1>
  %1 = tail call svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> %A, <16 x i1> %0, <16 x float> %C)
  ret <16 x float> %1
}

; CHECK-LABEL: @test_widen_fmf
; CHECK: [[ARG:%.*]] = shufflevector <4 x float> %A, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK:%.*]] = shufflevector <4 x i32> %B, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call nnan afn svml_avx_cc <8 x float> @__svml_sinf8_mask_e9(<8 x float> [[ARG]], <8 x i32> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <8 x float> [[RESULT]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x float> [[RESULT_EXTRACT]]

define <4 x float> @test_widen_fmf(<4 x float> %A, <4 x i32> %B) #1 {
entry:
  %0 = tail call nnan afn svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

; CHECK-LABEL: @test_widen_nofmf
; CHECK: [[ARG:%.*]] = shufflevector <4 x float> %A, <4 x float> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[MASK:%.*]] = shufflevector <4 x i32> %B, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK: [[RESULT:%.*]] = call svml_avx_cc <8 x float> @__svml_sinf8_ha_mask_e9(<8 x float> [[ARG]], <8 x i32> [[MASK]])
; CHECK: [[RESULT_EXTRACT:%.*]] = shufflevector <8 x float> [[RESULT]], <8 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK: ret <4 x float> [[RESULT_EXTRACT]]

define <4 x float> @test_widen_nofmf(<4 x float> %A, <4 x i32> %B) #1 {
entry:
  %0 = tail call svml_cc <4 x float> @__svml_sinf4_mask(<4 x float> %A, <4 x i32> %B)
  ret <4 x float> %0
}

declare svml_cc <4 x float> @__svml_sinf4_mask(<4 x float>, <4 x i32>)
declare svml_cc <16 x float> @__svml_sinf16_mask(<16 x float>, <16 x i1>, <16 x float>)

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="512" "prefer-vector-width"="128" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="256" "prefer-vector-width"="256" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
