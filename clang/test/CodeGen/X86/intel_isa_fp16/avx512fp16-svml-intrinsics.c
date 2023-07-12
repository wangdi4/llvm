// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx512fp16 -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefix=CHECK-AVX512FP16

#include <immintrin.h>

// AVX512 half precision
#ifdef __AVX512FP16__
__m128h test_mm_cbrt_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cbrt_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_cbrts8(<8 x half> %{{.*}})
  return _mm_cbrt_ph(A);
}

__m256h test_mm256_cbrt_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cbrt_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_cbrts16(<16 x half> %{{.*}})
  return _mm256_cbrt_ph(A);
}

__m512h test_mm512_cbrt_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cbrt_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cbrts32(<32 x half> %{{.*}})
  return _mm512_cbrt_ph(A);
}

__m512h test_mm512_mask_cbrt_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cbrt_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cbrts32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cbrt_ph(A, B, C);
}

__m128h test_mm_exp_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_exp_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_exps8(<8 x half> %{{.*}})
  return _mm_exp_ph(A);
}

__m256h test_mm256_exp_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_exp_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_exps16(<16 x half> %{{.*}})
  return _mm256_exp_ph(A);
}

__m512h test_mm512_exp_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_exp_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exps32(<32 x half> %{{.*}})
  return _mm512_exp_ph(A);
}

__m512h test_mm512_mask_exp_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_exp_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exps32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_exp_ph(A, B, C);
}

__m128h test_mm_exp10_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_exp10_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_exp10s8(<8 x half> %{{.*}})
  return _mm_exp10_ph(A);
}

__m256h test_mm256_exp10_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_exp10_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_exp10s16(<16 x half> %{{.*}})
  return _mm256_exp10_ph(A);
}

__m512h test_mm512_exp10_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_exp10_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exp10s32(<32 x half> %{{.*}})
  return _mm512_exp10_ph(A);
}

__m512h test_mm512_mask_exp10_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_exp10_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exp10s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_exp10_ph(A, B, C);
}

__m128h test_mm_exp2_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_exp2_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_exp2s8(<8 x half> %{{.*}})
  return _mm_exp2_ph(A);
}

__m256h test_mm256_exp2_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_exp2_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_exp2s16(<16 x half> %{{.*}})
  return _mm256_exp2_ph(A);
}

__m512h test_mm512_exp2_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_exp2_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exp2s32(<32 x half> %{{.*}})
  return _mm512_exp2_ph(A);
}

__m512h test_mm512_mask_exp2_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_exp2_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_exp2s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_exp2_ph(A, B, C);
}

__m128h test_mm_expm1_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_expm1_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_expm1s8(<8 x half> %{{.*}})
  return _mm_expm1_ph(A);
}

__m256h test_mm256_expm1_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_expm1_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_expm1s16(<16 x half> %{{.*}})
  return _mm256_expm1_ph(A);
}

__m512h test_mm512_expm1_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_expm1_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_expm1s32(<32 x half> %{{.*}})
  return _mm512_expm1_ph(A);
}

__m512h test_mm512_mask_expm1_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_expm1_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_expm1s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_expm1_ph(A, B, C);
}

__m512h test_mm512_hypot_ph(__m512h A, __m512h B) {
  // CHECK-AVX512FP16-LABEL: test_mm512_hypot_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_hypots32(<32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_hypot_ph(A, B);
}

__m512h test_mm512_mask_hypot_ph(__m512h A, __mmask32 B, __m512h C, __m512h D) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_hypot_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_hypots32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_mask_hypot_ph(A, B, C, D);
}

__m128h test_mm_invcbrt_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_invcbrt_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_invcbrts8(<8 x half> %{{.*}})
  return _mm_invcbrt_ph(A);
}

__m256h test_mm256_invcbrt_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_invcbrt_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_invcbrts16(<16 x half> %{{.*}})
  return _mm256_invcbrt_ph(A);
}

__m128h test_mm_invsqrt_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_invsqrt_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_invsqrts8(<8 x half> %{{.*}})
  return _mm_invsqrt_ph(A);
}

__m256h test_mm256_invsqrt_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_invsqrt_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_invsqrts16(<16 x half> %{{.*}})
  return _mm256_invsqrt_ph(A);
}

__m512h test_mm512_invsqrt_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_invsqrt_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_invsqrts32(<32 x half> %{{.*}})
  return _mm512_invsqrt_ph(A);
}

__m512h test_mm512_mask_invsqrt_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_invsqrt_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_invsqrts32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_invsqrt_ph(A, B, C);
}

__m128h test_mm_log_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_log_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_logs8(<8 x half> %{{.*}})
  return _mm_log_ph(A);
}

__m256h test_mm256_log_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_log_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_logs16(<16 x half> %{{.*}})
  return _mm256_log_ph(A);
}

__m512h test_mm512_log_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_log_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_logs32(<32 x half> %{{.*}})
  return _mm512_log_ph(A);
}

__m512h test_mm512_mask_log_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_log_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_logs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_log_ph(A, B, C);
}

__m128h test_mm_log10_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_log10_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_log10s8(<8 x half> %{{.*}})
  return _mm_log10_ph(A);
}

__m256h test_mm256_log10_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_log10_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_log10s16(<16 x half> %{{.*}})
  return _mm256_log10_ph(A);
}

__m512h test_mm512_log10_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_log10_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log10s32(<32 x half> %{{.*}})
  return _mm512_log10_ph(A);
}

__m512h test_mm512_mask_log10_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_log10_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log10s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_log10_ph(A, B, C);
}

__m128h test_mm_log1p_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_log1p_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_log1ps8(<8 x half> %{{.*}})
  return _mm_log1p_ph(A);
}

__m256h test_mm256_log1p_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_log1p_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_log1ps16(<16 x half> %{{.*}})
  return _mm256_log1p_ph(A);
}

__m512h test_mm512_log1p_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_log1p_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log1ps32(<32 x half> %{{.*}})
  return _mm512_log1p_ph(A);
}

__m512h test_mm512_mask_log1p_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_log1p_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log1ps32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_log1p_ph(A, B, C);
}

__m128h test_mm_log2_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_log2_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_log2s8(<8 x half> %{{.*}})
  return _mm_log2_ph(A);
}

__m256h test_mm256_log2_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_log2_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_log2s16(<16 x half> %{{.*}})
  return _mm256_log2_ph(A);
}

__m512h test_mm512_log2_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_log2_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log2s32(<32 x half> %{{.*}})
  return _mm512_log2_ph(A);
}

__m512h test_mm512_mask_log2_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_log2_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_log2s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_log2_ph(A, B, C);
}

__m128h test_mm_logb_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_logb_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_logbs8(<8 x half> %{{.*}})
  return _mm_logb_ph(A);
}

__m256h test_mm256_logb_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_logb_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_logbs16(<16 x half> %{{.*}})
  return _mm256_logb_ph(A);
}

__m512h test_mm512_logb_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_logb_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_logbs32(<32 x half> %{{.*}})
  return _mm512_logb_ph(A);
}

__m512h test_mm512_mask_logb_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_logb_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_logbs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_logb_ph(A, B, C);
}

__m128h test_mm_pow_ph(__m128h A, __m128h B) {
  // CHECK-AVX512FP16-LABEL: test_mm_pow_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_pows8(<8 x half> %{{.*}}, <8 x half> %{{.*}})
  return _mm_pow_ph(A, B);
}

__m256h test_mm256_pow_ph(__m256h A, __m256h B) {
  // CHECK-AVX512FP16-LABEL: test_mm256_pow_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_pows16(<16 x half> %{{.*}}, <16 x half> %{{.*}})
  return _mm256_pow_ph(A, B);
}

__m512h test_mm512_pow_ph(__m512h A, __m512h B) {
  // CHECK-AVX512FP16-LABEL: test_mm512_pow_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_pows32(<32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_pow_ph(A, B);
}

__m512h test_mm512_mask_pow_ph(__m512h A, __mmask32 B, __m512h C, __m512h D) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_pow_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_pows32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_mask_pow_ph(A, B, C, D);
}

__m512h test_mm512_recip_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_recip_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rcp.ph.512(<32 x half> %{{.*}}, <32 x half> zeroinitializer, i32 -1)
  return _mm512_recip_ph(A);
}

__m512h test_mm512_mask_recip_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_recip_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rcp.ph.512(<32 x half> %{{.*}}, <32 x half> %{{.*}}, i32 %8)
  return _mm512_mask_recip_ph(A, B, C);
}

__m128h test_mm_svml_sqrt_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_svml_sqrt_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_sqrts8(<8 x half> %{{.*}})
  return _mm_svml_sqrt_ph(A);
}

__m256h test_mm256_svml_sqrt_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_svml_sqrt_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_sqrts16(<16 x half> %{{.*}})
  return _mm256_svml_sqrt_ph(A);
}

__m128h test_mm_acos_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_acos_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_acoss8(<8 x half> %{{.*}})
  return _mm_acos_ph(A);
}

__m256h test_mm256_acos_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_acos_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_acoss16(<16 x half> %{{.*}})
  return _mm256_acos_ph(A);
}

__m512h test_mm512_acos_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_acos_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_acoss32(<32 x half> %{{.*}})
  return _mm512_acos_ph(A);
}

__m512h test_mm512_mask_acos_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_acos_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_acoss32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_acos_ph(A, B, C);
}

__m128h test_mm_acosh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_acosh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_acoshs8(<8 x half> %{{.*}})
  return _mm_acosh_ph(A);
}

__m256h test_mm256_acosh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_acosh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_acoshs16(<16 x half> %{{.*}})
  return _mm256_acosh_ph(A);
}

__m512h test_mm512_acosh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_acosh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_acoshs32(<32 x half> %{{.*}})
  return _mm512_acosh_ph(A);
}

__m512h test_mm512_mask_acosh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_acosh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_acoshs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_acosh_ph(A, B, C);
}

__m128h test_mm_asin_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_asin_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_asins8(<8 x half> %{{.*}})
  return _mm_asin_ph(A);
}

__m256h test_mm256_asin_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_asin_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_asins16(<16 x half> %{{.*}})
  return _mm256_asin_ph(A);
}

__m512h test_mm512_asin_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_asin_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_asins32(<32 x half> %{{.*}})
  return _mm512_asin_ph(A);
}

__m512h test_mm512_mask_asin_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_asin_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_asins32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_asin_ph(A, B, C);
}

__m128h test_mm_asinh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_asinh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_asinhs8(<8 x half> %{{.*}})
  return _mm_asinh_ph(A);
}

__m256h test_mm256_asinh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_asinh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_asinhs16(<16 x half> %{{.*}})
  return _mm256_asinh_ph(A);
}

__m512h test_mm512_asinh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_asinh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_asinhs32(<32 x half> %{{.*}})
  return _mm512_asinh_ph(A);
}

__m512h test_mm512_mask_asinh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_asinh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_asinhs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_asinh_ph(A, B, C);
}

__m128h test_mm_atan_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_atan_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_atans8(<8 x half> %{{.*}})
  return _mm_atan_ph(A);
}

__m256h test_mm256_atan_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_atan_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_atans16(<16 x half> %{{.*}})
  return _mm256_atan_ph(A);
}

__m512h test_mm512_atan_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_atan_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atans32(<32 x half> %{{.*}})
  return _mm512_atan_ph(A);
}

__m512h test_mm512_mask_atan_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_atan_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atans32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_atan_ph(A, B, C);
}

__m128h test_mm_atan2_ph(__m128h A, __m128h B) {
  // CHECK-AVX512FP16-LABEL: test_mm_atan2_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_atan2s8(<8 x half> %{{.*}}, <8 x half> %{{.*}})
  return _mm_atan2_ph(A, B);
}

__m256h test_mm256_atan2_ph(__m256h A, __m256h B) {
  // CHECK-AVX512FP16-LABEL: test_mm256_atan2_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_atan2s16(<16 x half> %{{.*}}, <16 x half> %{{.*}})
  return _mm256_atan2_ph(A, B);
}

__m512h test_mm512_atan2_ph(__m512h A, __m512h B) {
  // CHECK-AVX512FP16-LABEL: test_mm512_atan2_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atan2s32(<32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_atan2_ph(A, B);
}

__m512h test_mm512_mask_atan2_ph(__m512h A, __mmask32 B, __m512h C, __m512h D) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_atan2_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atan2s32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}}, <32 x half> %{{.*}})
  return _mm512_mask_atan2_ph(A, B, C, D);
}

__m128h test_mm_atanh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_atanh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_atanhs8(<8 x half> %{{.*}})
  return _mm_atanh_ph(A);
}

__m256h test_mm256_atanh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_atanh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_atanhs16(<16 x half> %{{.*}})
  return _mm256_atanh_ph(A);
}

__m512h test_mm512_atanh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_atanh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atanhs32(<32 x half> %{{.*}})
  return _mm512_atanh_ph(A);
}

__m512h test_mm512_mask_atanh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_atanh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_atanhs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_atanh_ph(A, B, C);
}

__m128h test_mm_cos_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cos_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_coss8(<8 x half> %{{.*}})
  return _mm_cos_ph(A);
}

__m256h test_mm256_cos_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cos_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_coss16(<16 x half> %{{.*}})
  return _mm256_cos_ph(A);
}

__m512h test_mm512_cos_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cos_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_coss32(<32 x half> %{{.*}})
  return _mm512_cos_ph(A);
}

__m512h test_mm512_mask_cos_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cos_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_coss32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cos_ph(A, B, C);
}

__m128h test_mm_cosd_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cosd_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_cosds8(<8 x half> %{{.*}})
  return _mm_cosd_ph(A);
}

__m256h test_mm256_cosd_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cosd_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_cosds16(<16 x half> %{{.*}})
  return _mm256_cosd_ph(A);
}

__m512h test_mm512_cosd_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cosd_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cosds32(<32 x half> %{{.*}})
  return _mm512_cosd_ph(A);
}

__m512h test_mm512_mask_cosd_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cosd_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cosds32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cosd_ph(A, B, C);
}

__m128h test_mm_cosh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cosh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_coshs8(<8 x half> %{{.*}})
  return _mm_cosh_ph(A);
}

__m256h test_mm256_cosh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cosh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_coshs16(<16 x half> %{{.*}})
  return _mm256_cosh_ph(A);
}

__m512h test_mm512_cosh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cosh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_coshs32(<32 x half> %{{.*}})
  return _mm512_cosh_ph(A);
}

__m512h test_mm512_mask_cosh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cosh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_coshs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cosh_ph(A, B, C);
}

__m128h test_mm_hypot_ph(__m128h A, __m128h B) {
  // CHECK-AVX512FP16-LABEL: test_mm_hypot_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_hypots8(<8 x half> %{{.*}}, <8 x half> %{{.*}})
  return _mm_hypot_ph(A, B);
}

__m256h test_mm256_hypot_ph(__m256h A, __m256h B) {
  // CHECK-AVX512FP16-LABEL: test_mm256_hypot_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_hypots16(<16 x half> %{{.*}}, <16 x half> %{{.*}})
  return _mm256_hypot_ph(A, B);
}

__m128h test_mm_sin_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_sin_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_sins8(<8 x half> %{{.*}})
  return _mm_sin_ph(A);
}

__m256h test_mm256_sin_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_sin_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_sins16(<16 x half> %{{.*}})
  return _mm256_sin_ph(A);
}

__m512h test_mm512_sin_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_sin_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sins32(<32 x half> %{{.*}})
  return _mm512_sin_ph(A);
}

__m512h test_mm512_mask_sin_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_sin_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sins32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_sin_ph(A, B, C);
}

__m128h test_mm_sincos_ph(__m128h *A, __m128h B) {
  // CHECK-AVX512FP16-LABEL: test_mm_sincos_ph
  // CHECK-AVX512FP16: [[RESULT:%.*]] = call svml_cc { <8 x half>, <8 x half> } @__svml_sincoss8(<8 x half> %{{.*}})
  // CHECK-AVX512FP16: [[COS:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT]], 1
  // CHECK-AVX512FP16: store <8 x half> [[COS]], ptr %{{.*}}
  // CHECK-AVX512FP16: [[SIN:%.*]] = extractvalue { <8 x half>, <8 x half> } [[RESULT]], 0
  // CHECK-AVX512FP16: ret <8 x half> [[SIN]]
  return _mm_sincos_ph(A, B);
}

__m256h test_mm256_sincos_ph(__m256h *A, __m256h B) {
  // CHECK-AVX512FP16-LABEL: test_mm256_sincos_ph
  // CHECK-AVX512FP16: [[RESULT:%.*]] = call svml_cc { <16 x half>, <16 x half> } @__svml_sincoss16(<16 x half> %{{.*}})
  // CHECK-AVX512FP16: [[COS:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 1
  // CHECK-AVX512FP16: store <16 x half> [[COS]], ptr %{{.*}}
  // CHECK-AVX512FP16: [[SIN:%.*]] = extractvalue { <16 x half>, <16 x half> } [[RESULT]], 0
  // CHECK-AVX512FP16: ret <16 x half> [[SIN]]
  return _mm256_sincos_ph(A, B);
}

__m512h test_mm512_sincos_ph(__m512h *A, __m512h B) {
  // CHECK-AVX512FP16-LABEL: test_mm512_sincos_ph
  // CHECK-AVX512FP16: [[RESULT:%.*]] = call svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32(<32 x half> %{{.*}})
  // CHECK-AVX512FP16: [[COS:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
  // CHECK-AVX512FP16: store <32 x half> [[COS]], ptr %{{.*}}
  // CHECK-AVX512FP16: [[SIN:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
  // CHECK-AVX512FP16: ret <32 x half> [[SIN]]
  return _mm512_sincos_ph(A, B);
}

__m512h test_mm512_mask_sincos_ph(__m512h* A, __m512h B, __m512h C, __mmask32 D, __m512h E) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_sincos_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: [[SRC_TMP:%.*]] = insertvalue { <32 x half>, <32 x half> } undef, <32 x half> %{{.*}}, 0
  // CHECK-AVX512FP16: [[SRC:%.*]] = insertvalue { <32 x half>, <32 x half> } [[SRC_TMP]], <32 x half> %{{.*}}, 1
  // CHECK-AVX512FP16: [[RESULT:%.*]] = call svml_cc { <32 x half>, <32 x half> } @__svml_sincoss32_mask({ <32 x half>, <32 x half> } [[SRC]], <32 x i1> [[MASK]], <32 x half> %{{.*}})
  // CHECK-AVX512FP16: [[COS:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 1
  // CHECK-AVX512FP16: store <32 x half> [[COS]], ptr %{{.*}}
  // CHECK-AVX512FP16: [[SIN:%.*]] = extractvalue { <32 x half>, <32 x half> } [[RESULT]], 0
  // CHECK-AVX512FP16: ret <32 x half> [[SIN]]
  return _mm512_mask_sincos_ph(A, B, C, D, E);
}

__m128h test_mm_sind_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_sind_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_sinds8(<8 x half> %{{.*}})
  return _mm_sind_ph(A);
}

__m256h test_mm256_sind_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_sind_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_sinds16(<16 x half> %{{.*}})
  return _mm256_sind_ph(A);
}

__m512h test_mm512_sind_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_sind_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sinds32(<32 x half> %{{.*}})
  return _mm512_sind_ph(A);
}

__m512h test_mm512_mask_sind_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_sind_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sinds32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_sind_ph(A, B, C);
}

__m128h test_mm_sinh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_sinh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_sinhs8(<8 x half> %{{.*}})
  return _mm_sinh_ph(A);
}

__m256h test_mm256_sinh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_sinh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_sinhs16(<16 x half> %{{.*}})
  return _mm256_sinh_ph(A);
}

__m512h test_mm512_sinh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_sinh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sinhs32(<32 x half> %{{.*}})
  return _mm512_sinh_ph(A);
}

__m512h test_mm512_mask_sinh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_sinh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_sinhs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_sinh_ph(A, B, C);
}

__m128h test_mm_tan_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_tan_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_tans8(<8 x half> %{{.*}})
  return _mm_tan_ph(A);
}

__m256h test_mm256_tan_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_tan_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_tans16(<16 x half> %{{.*}})
  return _mm256_tan_ph(A);
}

__m512h test_mm512_tan_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_tan_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tans32(<32 x half> %{{.*}})
  return _mm512_tan_ph(A);
}

__m512h test_mm512_mask_tan_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_tan_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tans32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_tan_ph(A, B, C);
}

__m128h test_mm_tand_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_tand_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_tands8(<8 x half> %{{.*}})
  return _mm_tand_ph(A);
}

__m256h test_mm256_tand_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_tand_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_tands16(<16 x half> %{{.*}})
  return _mm256_tand_ph(A);
}

__m512h test_mm512_tand_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_tand_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tands32(<32 x half> %{{.*}})
  return _mm512_tand_ph(A);
}

__m512h test_mm512_mask_tand_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_tand_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tands32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_tand_ph(A, B, C);
}

__m128h test_mm_tanh_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_tanh_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_tanhs8(<8 x half> %{{.*}})
  return _mm_tanh_ph(A);
}

__m256h test_mm256_tanh_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_tanh_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_tanhs16(<16 x half> %{{.*}})
  return _mm256_tanh_ph(A);
}

__m512h test_mm512_tanh_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_tanh_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tanhs32(<32 x half> %{{.*}})
  return _mm512_tanh_ph(A);
}

__m512h test_mm512_mask_tanh_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_tanh_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_tanhs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_tanh_ph(A, B, C);
}

__m128h test_mm_cdfnorm_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cdfnorm_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_cdfnorms8(<8 x half> %{{.*}})
  return _mm_cdfnorm_ph(A);
}

__m256h test_mm256_cdfnorm_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cdfnorm_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_cdfnorms16(<16 x half> %{{.*}})
  return _mm256_cdfnorm_ph(A);
}

__m512h test_mm512_cdfnorm_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cdfnorm_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cdfnorms32(<32 x half> %{{.*}})
  return _mm512_cdfnorm_ph(A);
}

__m512h test_mm512_mask_cdfnorm_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cdfnorm_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cdfnorms32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cdfnorm_ph(A, B, C);
}

__m128h test_mm_cdfnorminv_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_cdfnorminv_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_cdfnorminvs8(<8 x half> %{{.*}})
  return _mm_cdfnorminv_ph(A);
}

__m256h test_mm256_cdfnorminv_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_cdfnorminv_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_cdfnorminvs16(<16 x half> %{{.*}})
  return _mm256_cdfnorminv_ph(A);
}

__m512h test_mm512_cdfnorminv_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_cdfnorminv_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cdfnorminvs32(<32 x half> %{{.*}})
  return _mm512_cdfnorminv_ph(A);
}

__m512h test_mm512_mask_cdfnorminv_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_cdfnorminv_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_cdfnorminvs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_cdfnorminv_ph(A, B, C);
}

__m128h test_mm_erf_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_erf_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_erfs8(<8 x half> %{{.*}})
  return _mm_erf_ph(A);
}

__m256h test_mm256_erf_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_erf_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_erfs16(<16 x half> %{{.*}})
  return _mm256_erf_ph(A);
}

__m512h test_mm512_erf_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_erf_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfs32(<32 x half> %{{.*}})
  return _mm512_erf_ph(A);
}

__m512h test_mm512_mask_erf_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_erf_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_erf_ph(A, B, C);
}

__m128h test_mm_erfc_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_erfc_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_erfcs8(<8 x half> %{{.*}})
  return _mm_erfc_ph(A);
}

__m256h test_mm256_erfc_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_erfc_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_erfcs16(<16 x half> %{{.*}})
  return _mm256_erfc_ph(A);
}

__m512h test_mm512_erfc_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_erfc_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfcs32(<32 x half> %{{.*}})
  return _mm512_erfc_ph(A);
}

__m512h test_mm512_mask_erfc_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_erfc_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfcs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_erfc_ph(A, B, C);
}

__m128h test_mm_erfcinv_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_erfcinv_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_erfcinvs8(<8 x half> %{{.*}})
  return _mm_erfcinv_ph(A);
}

__m256h test_mm256_erfcinv_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_erfcinv_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_erfcinvs16(<16 x half> %{{.*}})
  return _mm256_erfcinv_ph(A);
}

__m512h test_mm512_erfcinv_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_erfcinv_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfcinvs32(<32 x half> %{{.*}})
  return _mm512_erfcinv_ph(A);
}

__m512h test_mm512_mask_erfcinv_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_erfcinv_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfcinvs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_erfcinv_ph(A, B, C);
}

__m128h test_mm_erfinv_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_erfinv_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_erfinvs8(<8 x half> %{{.*}})
  return _mm_erfinv_ph(A);
}

__m256h test_mm256_erfinv_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_erfinv_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_erfinvs16(<16 x half> %{{.*}})
  return _mm256_erfinv_ph(A);
}

__m512h test_mm512_erfinv_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_erfinv_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfinvs32(<32 x half> %{{.*}})
  return _mm512_erfinv_ph(A);
}

__m512h test_mm512_mask_erfinv_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_erfinv_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_erfinvs32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_erfinv_ph(A, B, C);
}

__m128h test_mm_svml_ceil_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_svml_ceil_ph
  // CHECK-AVX512FP16: call <8 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.128(<8 x half> %{{.*}}, i32 2, <8 x half> {{.*}}, i8 -1)
  return _mm_svml_ceil_ph(A);
}

__m256h test_mm256_svml_ceil_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_svml_ceil_ph
  // CHECK-AVX512FP16: call <16 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.256(<16 x half> %{{.*}}, i32 2, <16 x half> {{.*}}, i16 -1)
  return _mm256_svml_ceil_ph(A);
}

__m512h test_mm512_ceil_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_ceil_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 2, <32 x half> {{.*}}, i32 -1, i32 4)
  return _mm512_ceil_ph(A);
}

__m512h test_mm512_mask_ceil_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_ceil_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 2, <32 x half> {{.*}}, i32 %{{.*}}, i32 4)
  return _mm512_mask_ceil_ph(A, B, C);
}

__m128h test_mm_svml_floor_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_svml_floor_ph
  // CHECK-AVX512FP16: call <8 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.128(<8 x half> %{{.*}}, i32 1, <8 x half> {{.*}}, i8 -1)
  return _mm_svml_floor_ph(A);
}

__m256h test_mm256_svml_floor_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_svml_floor_ph
  // CHECK-AVX512FP16: call <16 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.256(<16 x half> %{{.*}}, i32 1, <16 x half> {{.*}}, i16 -1)
  return _mm256_svml_floor_ph(A);
}

__m512h test_mm512_floor_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_floor_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 1, <32 x half> {{.*}}, i32 -1, i32 4)
  return _mm512_floor_ph(A);
}

__m512h test_mm512_mask_floor_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_floor_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 1, <32 x half> {{.*}}, i32 %{{.*}}, i32 4)
  return _mm512_mask_floor_ph(A, B, C);
}

__m512h test_mm512_nearbyint_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_nearbyint_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_nearbyints32(<32 x half> %{{.*}})
  return _mm512_nearbyint_ph(A);
}

__m512h test_mm512_mask_nearbyint_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_nearbyint_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_nearbyints32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_nearbyint_ph(A, B, C);
}

__m512h test_mm512_rint_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_rint_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_rints32(<32 x half> %{{.*}})
  return _mm512_rint_ph(A);
}

__m512h test_mm512_mask_rint_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_rint_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_rints32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_rint_ph(A, B, C);
}

__m128h test_mm_svml_round_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_svml_round_ph
  // CHECK-AVX512FP16: call svml_cc <8 x half> @__svml_rounds8(<8 x half> %{{.*}})
  return _mm_svml_round_ph(A);
}

__m256h test_mm256_svml_round_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_svml_round_ph
  // CHECK-AVX512FP16: call svml_cc <16 x half> @__svml_rounds16(<16 x half> %{{.*}})
  return _mm256_svml_round_ph(A);
}

__m512h test_mm512_svml_round_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_svml_round_ph
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_rounds32(<32 x half> %{{.*}})
  return _mm512_svml_round_ph(A);
}

__m512h test_mm512_mask_svml_round_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_svml_round_ph
  // CHECK-AVX512FP16: [[MASK:%.*]] = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK-AVX512FP16: call svml_cc <32 x half> @__svml_rounds32_mask(<32 x half> %{{.*}}, <32 x i1> [[MASK]], <32 x half> %{{.*}})
  return _mm512_mask_svml_round_ph(A, B, C);
}

__m128h test_mm_trunc_ph(__m128h A) {
  // CHECK-AVX512FP16-LABEL: test_mm_trunc_ph
  // CHECK-AVX512FP16: call <8 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.128(<8 x half> %{{.*}}, i32 3, <8 x half> {{.*}}, i8 -1)
  return _mm_trunc_ph(A);
}

__m256h test_mm256_trunc_ph(__m256h A) {
  // CHECK-AVX512FP16-LABEL: test_mm256_trunc_ph
  // CHECK-AVX512FP16: call <16 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.256(<16 x half> %{{.*}}, i32 3, <16 x half> {{.*}}, i16 -1)
  return _mm256_trunc_ph(A);
}

__m512h test_mm512_trunc_ph(__m512h A) {
  // CHECK-AVX512FP16-LABEL: test_mm512_trunc_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 3, <32 x half> {{.*}}, i32 -1, i32 4)
  return _mm512_trunc_ph(A);
}

__m512h test_mm512_mask_trunc_ph(__m512h A, __mmask32 B, __m512h C) {
  // CHECK-AVX512FP16-LABEL: test_mm512_mask_trunc_ph
  // CHECK-AVX512FP16: call <32 x half> @llvm.x86.avx512fp16.mask.rndscale.ph.512(<32 x half> %{{.*}}, i32 3, <32 x half> {{.*}}, i32 %{{.*}}, i32 4)
  return _mm512_mask_trunc_ph(A, B, C);
}
#endif // __AVX512FP16__
