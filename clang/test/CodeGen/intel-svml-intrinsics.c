// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +sse2 -emit-llvm -o - -Wall -Werror | FileCheck %s
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx -emit-llvm -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx2 -emit-llvm -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1,CHECK-AVX2
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx512f -emit-llvm -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1,CHECK-AVX2,CHECK-AVX512F

#include <immintrin.h>

__m128 test_mm_acosh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_acosh_ps
  // CHECK: call svml_cc <4 x float> @__svml_acoshf4(<4 x float> %{{.*}})
  return _mm_acosh_ps(A);
}

__m128 test_mm_acos_ps(__m128 A) {
  // CHECK-LABEL: test_mm_acos_ps
  // CHECK: call svml_cc <4 x float> @__svml_acosf4(<4 x float> %{{.*}})
  return _mm_acos_ps(A);
}

__m128 test_mm_asinh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_asinh_ps
  // CHECK: call svml_cc <4 x float> @__svml_asinhf4(<4 x float> %{{.*}})
  return _mm_asinh_ps(A);
}

__m128 test_mm_asin_ps(__m128 A) {
  // CHECK-LABEL: test_mm_asin_ps
  // CHECK: call svml_cc <4 x float> @__svml_asinf4(<4 x float> %{{.*}})
  return _mm_asin_ps(A);
}

__m128 test_mm_atan2_ps(__m128 A, __m128 B) {
  // CHECK-LABEL: test_mm_atan2_ps
  // CHECK: call svml_cc <4 x float> @__svml_atan2f4(<4 x float> %{{.*}}, <4 x float> %{{.*}})
  return _mm_atan2_ps(A, B);
}

__m128 test_mm_atanh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_atanh_ps
  // CHECK: call svml_cc <4 x float> @__svml_atanhf4(<4 x float> %{{.*}})
  return _mm_atanh_ps(A);
}

__m128 test_mm_atan_ps(__m128 A) {
  // CHECK-LABEL: test_mm_atan_ps
  // CHECK: call svml_cc <4 x float> @__svml_atanf4(<4 x float> %{{.*}})
  return _mm_atan_ps(A);
}

__m128 test_mm_cbrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cbrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_cbrtf4(<4 x float> %{{.*}})
  return _mm_cbrt_ps(A);
}

__m128 test_mm_cosh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cosh_ps
  // CHECK: call svml_cc <4 x float> @__svml_coshf4(<4 x float> %{{.*}})
  return _mm_cosh_ps(A);
}

__m128 test_mm_cos_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cos_ps
  // CHECK: call svml_cc <4 x float> @__svml_cosf4(<4 x float> %{{.*}})
  return _mm_cos_ps(A);
}

__m128 test_mm_erfc_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erfc_ps
  // CHECK: call svml_cc <4 x float> @__svml_erfcf4(<4 x float> %{{.*}})
  return _mm_erfc_ps(A);
}

__m128 test_mm_erfinv_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erfinv_ps
  // CHECK: call svml_cc <4 x float> @__svml_erfinvf4(<4 x float> %{{.*}})
  return _mm_erfinv_ps(A);
}

__m128 test_mm_erf_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erf_ps
  // CHECK: call svml_cc <4 x float> @__svml_erff4(<4 x float> %{{.*}})
  return _mm_erf_ps(A);
}

__m128 test_mm_exp2_ps(__m128 A) {
  // CHECK-LABEL: test_mm_exp2_ps
  // CHECK: call svml_cc <4 x float> @__svml_exp2f4(<4 x float> %{{.*}})
  return _mm_exp2_ps(A);
}

__m128 test_mm_exp_ps(__m128 A) {
  // CHECK-LABEL: test_mm_exp_ps
  // CHECK: call svml_cc <4 x float> @__svml_expf4(<4 x float> %{{.*}})
  return _mm_exp_ps(A);
}

__m128 test_mm_invcbrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_invcbrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_invcbrtf4(<4 x float> %{{.*}})
  return _mm_invcbrt_ps(A);
}

__m128 test_mm_invsqrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_invsqrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_invsqrtf4(<4 x float> %{{.*}})
  return _mm_invsqrt_ps(A);
}

__m128 test_mm_log10_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log10_ps
  // CHECK: call svml_cc <4 x float> @__svml_log10f4(<4 x float> %{{.*}})
  return _mm_log10_ps(A);
}

__m128 test_mm_log2_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log2_ps
  // CHECK: call svml_cc <4 x float> @__svml_log2f4(<4 x float> %{{.*}})
  return _mm_log2_ps(A);
}

__m128 test_mm_log_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log_ps
  // CHECK: call svml_cc <4 x float> @__svml_logf4(<4 x float> %{{.*}})
  return _mm_log_ps(A);
}

__m128 test_mm_pow_ps(__m128 A, __m128 B) {
  // CHECK-LABEL: test_mm_pow_ps
  // CHECK: call svml_cc <4 x float> @__svml_powf4(<4 x float> %{{.*}}, <4 x float> %{{.*}})
  return _mm_pow_ps(A, B);
}

__m128 test_mm_sinh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_sinh_ps
  // CHECK: call svml_cc <4 x float> @__svml_sinhf4(<4 x float> %{{.*}})
  return _mm_sinh_ps(A);
}

__m128 test_mm_sin_ps(__m128 A) {
  // CHECK-LABEL: test_mm_sin_ps
  // CHECK: call svml_cc <4 x float> @__svml_sinf4(<4 x float> %{{.*}})
  return _mm_sin_ps(A);
}

__m128 test_mm_tanh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_tanh_ps
  // CHECK: call svml_cc <4 x float> @__svml_tanhf4(<4 x float> %{{.*}})
  return _mm_tanh_ps(A);
}

__m128 test_mm_tan_ps(__m128 A) {
  // CHECK-LABEL: test_mm_tan_ps
  // CHECK: call svml_cc <4 x float> @__svml_tanf4(<4 x float> %{{.*}})
  return _mm_tan_ps(A);
}

__m128d test_mm_acosh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_acosh_pd
  // CHECK: call svml_cc <2 x double> @__svml_acosh2(<2 x double> %{{.*}})
  return _mm_acosh_pd(A);
}

__m128d test_mm_acos_pd(__m128d A) {
  // CHECK-LABEL: test_mm_acos_pd
  // CHECK: call svml_cc <2 x double> @__svml_acos2(<2 x double> %{{.*}})
  return _mm_acos_pd(A);
}

__m128d test_mm_asinh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_asinh_pd
  // CHECK: call svml_cc <2 x double> @__svml_asinh2(<2 x double> %{{.*}})
  return _mm_asinh_pd(A);
}

__m128d test_mm_asin_pd(__m128d A) {
  // CHECK-LABEL: test_mm_asin_pd
  // CHECK: call svml_cc <2 x double> @__svml_asin2(<2 x double> %{{.*}})
  return _mm_asin_pd(A);
}

__m128d test_mm_atan2_pd(__m128d A, __m128d B) {
  // CHECK-LABEL: test_mm_atan2_pd
  // CHECK: call svml_cc <2 x double> @__svml_atan22(<2 x double> %{{.*}}, <2 x double> %{{.*}})
  return _mm_atan2_pd(A, B);
}

__m128d test_mm_atanh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_atanh_pd
  // CHECK: call svml_cc <2 x double> @__svml_atanh2(<2 x double> %{{.*}})
  return _mm_atanh_pd(A);
}

__m128d test_mm_atan_pd(__m128d A) {
  // CHECK-LABEL: test_mm_atan_pd
  // CHECK: call svml_cc <2 x double> @__svml_atan2(<2 x double> %{{.*}})
  return _mm_atan_pd(A);
}

__m128d test_mm_cbrt_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cbrt_pd
  // CHECK: call svml_cc <2 x double> @__svml_cbrt2(<2 x double> %{{.*}})
  return _mm_cbrt_pd(A);
}

__m128d test_mm_cosh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cosh_pd
  // CHECK: call svml_cc <2 x double> @__svml_cosh2(<2 x double> %{{.*}})
  return _mm_cosh_pd(A);
}

__m128d test_mm_cos_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cos_pd
  // CHECK: call svml_cc <2 x double> @__svml_cos2(<2 x double> %{{.*}})
  return _mm_cos_pd(A);
}

__m128d test_mm_erfc_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erfc_pd
  // CHECK: call svml_cc <2 x double> @__svml_erfc2(<2 x double> %{{.*}})
  return _mm_erfc_pd(A);
}

__m128d test_mm_erfinv_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erfinv_pd
  // CHECK: call svml_cc <2 x double> @__svml_erfinv2(<2 x double> %{{.*}})
  return _mm_erfinv_pd(A);
}

__m128d test_mm_erf_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erf_pd
  // CHECK: call svml_cc <2 x double> @__svml_erf2(<2 x double> %{{.*}})
  return _mm_erf_pd(A);
}

__m128d test_mm_exp2_pd(__m128d A) {
  // CHECK-LABEL: test_mm_exp2_pd
  // CHECK: call svml_cc <2 x double> @__svml_exp22(<2 x double> %{{.*}})
  return _mm_exp2_pd(A);
}

__m128d test_mm_exp_pd(__m128d A) {
  // CHECK-LABEL: test_mm_exp_pd
  // CHECK: call svml_cc <2 x double> @__svml_exp2(<2 x double> %{{.*}})
  return _mm_exp_pd(A);
}

__m128d test_mm_invcbrt_pd(__m128d A) {
  // CHECK-LABEL: test_mm_invcbrt_pd
  // CHECK: call svml_cc <2 x double> @__svml_invcbrt2(<2 x double> %{{.*}})
  return _mm_invcbrt_pd(A);
}

__m128d test_mm_invsqrt_pd(__m128d A) {
  // CHECK-LABEL: test_mm_invsqrt_pd
  // CHECK: call svml_cc <2 x double> @__svml_invsqrt2(<2 x double> %{{.*}})
  return _mm_invsqrt_pd(A);
}

__m128d test_mm_log10_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log10_pd
  // CHECK: call svml_cc <2 x double> @__svml_log102(<2 x double> %{{.*}})
  return _mm_log10_pd(A);
}

__m128d test_mm_log2_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log2_pd
  // CHECK: call svml_cc <2 x double> @__svml_log22(<2 x double> %{{.*}})
  return _mm_log2_pd(A);
}

__m128d test_mm_log_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log_pd
  // CHECK: call svml_cc <2 x double> @__svml_log2(<2 x double> %{{.*}})
  return _mm_log_pd(A);
}

__m128d test_mm_pow_pd(__m128d A, __m128d B) {
  // CHECK-LABEL: test_mm_pow_pd
  // CHECK: call svml_cc <2 x double> @__svml_pow2(<2 x double> %{{.*}}, <2 x double> %{{.*}})
  return _mm_pow_pd(A, B);
}

__m128d test_mm_sinh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_sinh_pd
  // CHECK: call svml_cc <2 x double> @__svml_sinh2(<2 x double> %{{.*}})
  return _mm_sinh_pd(A);
}

__m128d test_mm_sin_pd(__m128d A) {
  // CHECK-LABEL: test_mm_sin_pd
  // CHECK: call svml_cc <2 x double> @__svml_sin2(<2 x double> %{{.*}})
  return _mm_sin_pd(A);
}

__m128d test_mm_tanh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_tanh_pd
  // CHECK: call svml_cc <2 x double> @__svml_tanh2(<2 x double> %{{.*}})
  return _mm_tanh_pd(A);
}

__m128d test_mm_tan_pd(__m128d A) {
  // CHECK-LABEL: test_mm_tan_pd
  // CHECK: call svml_cc <2 x double> @__svml_tan2(<2 x double> %{{.*}})
  return _mm_tan_pd(A);
}

__m128i test_mm_irem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_irem_epi32
  // CHECK: call svml_cc <4 x i32> @__svml_irem4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_irem_epi32(A, B);
}

__m128i test_mm_idiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_idiv_epi32
  // CHECK: call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_idiv_epi32(A, B);
}

__m128i test_mm_urem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_urem_epi32
  // CHECK: call svml_cc <4 x i32> @__svml_urem4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_urem_epi32(A, B);
}

__m128i test_mm_udiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_udiv_epi32
  // CHECK: call svml_cc <4 x i32> @__svml_udiv4(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_udiv_epi32(A, B);
}

#ifdef __AVX__
__m256 test_mm256_acosh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_acosh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_acoshf8(<8 x float> %{{.*}})
  return _mm256_acosh_ps(A);
}

__m256 test_mm256_acos_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_acos_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_acosf8(<8 x float> %{{.*}})
  return _mm256_acos_ps(A);
}

__m256 test_mm256_asinh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_asinh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_asinhf8(<8 x float> %{{.*}})
  return _mm256_asinh_ps(A);
}

__m256 test_mm256_asin_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_asin_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_asinf8(<8 x float> %{{.*}})
  return _mm256_asin_ps(A);
}

__m256 test_mm256_atan2_ps(__m256 A, __m256 B) {
  // CHECK-AVX1-LABEL: test_mm256_atan2_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_atan2f8(<8 x float> %{{.*}}, <8 x float> %{{.*}})
  return _mm256_atan2_ps(A, B);
}

__m256 test_mm256_atanh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_atanh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_atanhf8(<8 x float> %{{.*}})
  return _mm256_atanh_ps(A);
}

__m256 test_mm256_atan_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_atan_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_atanf8(<8 x float> %{{.*}})
  return _mm256_atan_ps(A);
}

__m256 test_mm256_cbrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cbrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cbrtf8(<8 x float> %{{.*}})
  return _mm256_cbrt_ps(A);
}

__m256 test_mm256_cosh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cosh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_coshf8(<8 x float> %{{.*}})
  return _mm256_cosh_ps(A);
}

__m256 test_mm256_cos_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cos_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cosf8(<8 x float> %{{.*}})
  return _mm256_cos_ps(A);
}

__m256 test_mm256_erfc_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erfc_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erfcf8(<8 x float> %{{.*}})
  return _mm256_erfc_ps(A);
}

__m256 test_mm256_erfinv_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erfinv_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erfinvf8(<8 x float> %{{.*}})
  return _mm256_erfinv_ps(A);
}

__m256 test_mm256_erf_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erf_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erff8(<8 x float> %{{.*}})
  return _mm256_erf_ps(A);
}

__m256 test_mm256_exp2_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_exp2_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_exp2f8(<8 x float> %{{.*}})
  return _mm256_exp2_ps(A);
}

__m256 test_mm256_exp_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_exp_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_expf8(<8 x float> %{{.*}})
  return _mm256_exp_ps(A);
}

__m256 test_mm256_invcbrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_invcbrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_invcbrtf8(<8 x float> %{{.*}})
  return _mm256_invcbrt_ps(A);
}

__m256 test_mm256_invsqrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_invsqrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_invsqrtf8(<8 x float> %{{.*}})
  return _mm256_invsqrt_ps(A);
}

__m256 test_mm256_log10_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log10_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_log10f8(<8 x float> %{{.*}})
  return _mm256_log10_ps(A);
}

__m256 test_mm256_log2_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log2_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_log2f8(<8 x float> %{{.*}})
  return _mm256_log2_ps(A);
}

__m256 test_mm256_log_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_logf8(<8 x float> %{{.*}})
  return _mm256_log_ps(A);
}

__m256 test_mm256_pow_ps(__m256 A, __m256 B) {
  // CHECK-AVX1-LABEL: test_mm256_pow_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_powf8(<8 x float> %{{.*}}, <8 x float> %{{.*}})
  return _mm256_pow_ps(A, B);
}

__m256 test_mm256_sinh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_sinh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sinhf8(<8 x float> %{{.*}})
  return _mm256_sinh_ps(A);
}

__m256 test_mm256_sin_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_sin_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sinf8(<8 x float> %{{.*}})
  return _mm256_sin_ps(A);
}

__m256 test_mm256_tanh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_tanh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_tanhf8(<8 x float> %{{.*}})
  return _mm256_tanh_ps(A);
}

__m256 test_mm256_tan_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_tan_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_tanf8(<8 x float> %{{.*}})
  return _mm256_tan_ps(A);
}

__m256d test_mm256_acosh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_acosh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_acosh4(<4 x double> %{{.*}})
  return _mm256_acosh_pd(A);
}

__m256d test_mm256_acos_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_acos_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_acos4(<4 x double> %{{.*}})
  return _mm256_acos_pd(A);
}

__m256d test_mm256_asinh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_asinh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_asinh4(<4 x double> %{{.*}})
  return _mm256_asinh_pd(A);
}

__m256d test_mm256_asin_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_asin_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_asin4(<4 x double> %{{.*}})
  return _mm256_asin_pd(A);
}

__m256d test_mm256_atan2_pd(__m256d A, __m256d B) {
  // CHECK-AVX1-LABEL: test_mm256_atan2_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_atan24(<4 x double> %{{.*}}, <4 x double> %{{.*}})
  return _mm256_atan2_pd(A, B);
}

__m256d test_mm256_atanh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_atanh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_atanh4(<4 x double> %{{.*}})
  return _mm256_atanh_pd(A);
}

__m256d test_mm256_atan_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_atan_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_atan4(<4 x double> %{{.*}})
  return _mm256_atan_pd(A);
}

__m256d test_mm256_cbrt_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cbrt_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cbrt4(<4 x double> %{{.*}})
  return _mm256_cbrt_pd(A);
}

__m256d test_mm256_cosh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cosh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cosh4(<4 x double> %{{.*}})
  return _mm256_cosh_pd(A);
}

__m256d test_mm256_cos_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cos_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cos4(<4 x double> %{{.*}})
  return _mm256_cos_pd(A);
}

__m256d test_mm256_erfc_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erfc_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erfc4(<4 x double> %{{.*}})
  return _mm256_erfc_pd(A);
}

__m256d test_mm256_erfinv_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erfinv_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erfinv4(<4 x double> %{{.*}})
  return _mm256_erfinv_pd(A);
}

__m256d test_mm256_erf_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erf_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erf4(<4 x double> %{{.*}})
  return _mm256_erf_pd(A);
}

__m256d test_mm256_exp2_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_exp2_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_exp24(<4 x double> %{{.*}})
  return _mm256_exp2_pd(A);
}

__m256d test_mm256_exp_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_exp_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_exp4(<4 x double> %{{.*}})
  return _mm256_exp_pd(A);
}

__m256d test_mm256_invcbrt_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_invcbrt_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_invcbrt4(<4 x double> %{{.*}})
  return _mm256_invcbrt_pd(A);
}

__m256d test_mm256_invsqrt_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_invsqrt_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_invsqrt4(<4 x double> %{{.*}})
  return _mm256_invsqrt_pd(A);
}

__m256d test_mm256_log10_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log10_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log104(<4 x double> %{{.*}})
  return _mm256_log10_pd(A);
}

__m256d test_mm256_log2_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log2_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log24(<4 x double> %{{.*}})
  return _mm256_log2_pd(A);
}

__m256d test_mm256_log_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log4(<4 x double> %{{.*}})
  return _mm256_log_pd(A);
}

__m256d test_mm256_pow_pd(__m256d A, __m256d B) {
  // CHECK-AVX1-LABEL: test_mm256_pow_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_pow4(<4 x double> %{{.*}}, <4 x double> %{{.*}})
  return _mm256_pow_pd(A, B);
}

__m256d test_mm256_sinh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_sinh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sinh4(<4 x double> %{{.*}})
  return _mm256_sinh_pd(A);
}

__m256d test_mm256_sin_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_sin_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sin4(<4 x double> %{{.*}})
  return _mm256_sin_pd(A);
}

__m256d test_mm256_tanh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_tanh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_tanh4(<4 x double> %{{.*}})
  return _mm256_tanh_pd(A);
}

__m256d test_mm256_tan_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_tan_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_tan4(<4 x double> %{{.*}})
  return _mm256_tan_pd(A);
}
#endif // __AVX__

#ifdef __AVX2__
__m256i test_mm256_rem_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi32
  // CHECK-AVX2: call svml_cc <8 x i32> @__svml_irem8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_rem_epi32(A, B);
}

__m256i test_mm256_div_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi32
  // CHECK-AVX2: call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_div_epi32(A, B);
}

__m256i test_mm256_rem_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu32
  // CHECK-AVX2: call svml_cc <8 x i32> @__svml_urem8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_rem_epu32(A, B);
}

__m256i test_mm256_div_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu32
  // CHECK-AVX2: call svml_cc <8 x i32> @__svml_udiv8(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_div_epu32(A, B);
}
#endif // __AVX2__

#ifdef __AVX512F__


__m512 test_mm512_acosh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_acosh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acoshf16(<16 x float> %{{.*}})
  return _mm512_acosh_ps(A);
}

__m512 test_mm512_acos_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_acos_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acosf16(<16 x float> %{{.*}})
  return _mm512_acos_ps(A);
}

__m512 test_mm512_asinh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_asinh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinhf16(<16 x float> %{{.*}})
  return _mm512_asinh_ps(A);
}

__m512 test_mm512_asin_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_asin_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinf16(<16 x float> %{{.*}})
  return _mm512_asin_ps(A);
}

__m512 test_mm512_atan2_ps(__m512 A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_atan2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atan2f16(<16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_atan2_ps(A, B);
}

__m512 test_mm512_atanh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_atanh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanhf16(<16 x float> %{{.*}})
  return _mm512_atanh_ps(A);
}

__m512 test_mm512_atan_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_atan_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanf16(<16 x float> %{{.*}})
  return _mm512_atan_ps(A);
}

__m512 test_mm512_cbrt_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cbrt_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cbrtf16(<16 x float> %{{.*}})
  return _mm512_cbrt_ps(A);
}

__m512 test_mm512_cosh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_coshf16(<16 x float> %{{.*}})
  return _mm512_cosh_ps(A);
}

__m512 test_mm512_cos_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cos_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cosf16(<16 x float> %{{.*}})
  return _mm512_cos_ps(A);
}

__m512 test_mm512_erfc_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfc_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfcf16(<16 x float> %{{.*}})
  return _mm512_erfc_ps(A);
}

__m512 test_mm512_erfinv_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfinv_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfinvf16(<16 x float> %{{.*}})
  return _mm512_erfinv_ps(A);
}

__m512 test_mm512_erf_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erf_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erff16(<16 x float> %{{.*}})
  return _mm512_erf_ps(A);
}

__m512 test_mm512_exp2_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_exp2f16(<16 x float> %{{.*}})
  return _mm512_exp2_ps(A);
}

__m512 test_mm512_exp_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_expf16(<16 x float> %{{.*}})
  return _mm512_exp_ps(A);
}

__m512 test_mm512_invcbrt_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_invcbrt_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_invcbrtf16(<16 x float> %{{.*}})
  return _mm512_invcbrt_ps(A);
}

__m512 test_mm512_invsqrt_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_invsqrt_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_invsqrtf16(<16 x float> %{{.*}})
  return _mm512_invsqrt_ps(A);
}

__m512 test_mm512_log10_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log10_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log10f16(<16 x float> %{{.*}})
  return _mm512_log10_ps(A);
}

__m512 test_mm512_log2_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log2f16(<16 x float> %{{.*}})
  return _mm512_log2_ps(A);
}

__m512 test_mm512_log_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_logf16(<16 x float> %{{.*}})
  return _mm512_log_ps(A);
}

__m512 test_mm512_pow_ps(__m512 A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_pow_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_powf16(<16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_pow_ps(A, B);
}

__m512 test_mm512_sinh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_sinh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinhf16(<16 x float> %{{.*}})
  return _mm512_sinh_ps(A);
}

__m512 test_mm512_sin_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_sin_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinf16(<16 x float> %{{.*}})
  return _mm512_sin_ps(A);
}

__m512 test_mm512_tanh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_tanh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanhf16(<16 x float> %{{.*}})
  return _mm512_tanh_ps(A);
}

__m512 test_mm512_tan_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_tan_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanf16(<16 x float> %{{.*}})
  return _mm512_tan_ps(A);
}

__m512d test_mm512_acosh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_acosh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acosh8(<8 x double> %{{.*}})
  return _mm512_acosh_pd(A);
}

__m512d test_mm512_acos_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_acos_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acos8(<8 x double> %{{.*}})
  return _mm512_acos_pd(A);
}

__m512d test_mm512_asinh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_asinh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asinh8(<8 x double> %{{.*}})
  return _mm512_asinh_pd(A);
}

__m512d test_mm512_asin_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_asin_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asin8(<8 x double> %{{.*}})
  return _mm512_asin_pd(A);
}

__m512d test_mm512_atan2_pd(__m512d A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_atan2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan28(<8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_atan2_pd(A, B);
}

__m512d test_mm512_atanh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_atanh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atanh8(<8 x double> %{{.*}})
  return _mm512_atanh_pd(A);
}

__m512d test_mm512_atan_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_atan_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan8(<8 x double> %{{.*}})
  return _mm512_atan_pd(A);
}

__m512d test_mm512_cbrt_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cbrt_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cbrt8(<8 x double> %{{.*}})
  return _mm512_cbrt_pd(A);
}

__m512d test_mm512_cosh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cosh8(<8 x double> %{{.*}})
  return _mm512_cosh_pd(A);
}

__m512d test_mm512_cos_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cos_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cos8(<8 x double> %{{.*}})
  return _mm512_cos_pd(A);
}

__m512d test_mm512_erfc_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfc_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfc8(<8 x double> %{{.*}})
  return _mm512_erfc_pd(A);
}

__m512d test_mm512_erfinv_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfinv_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfinv8(<8 x double> %{{.*}})
  return _mm512_erfinv_pd(A);
}

__m512d test_mm512_erf_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erf_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erf8(<8 x double> %{{.*}})
  return _mm512_erf_pd(A);
}

__m512d test_mm512_exp2_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp28(<8 x double> %{{.*}})
  return _mm512_exp2_pd(A);
}

__m512d test_mm512_exp_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp8(<8 x double> %{{.*}})
  return _mm512_exp_pd(A);
}

__m512d test_mm512_invcbrt_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_invcbrt_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_invcbrt8(<8 x double> %{{.*}})
  return _mm512_invcbrt_pd(A);
}

__m512d test_mm512_invsqrt_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_invsqrt_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_invsqrt8(<8 x double> %{{.*}})
  return _mm512_invsqrt_pd(A);
}

__m512d test_mm512_log10_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log10_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log108(<8 x double> %{{.*}})
  return _mm512_log10_pd(A);
}

__m512d test_mm512_log2_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log28(<8 x double> %{{.*}})
  return _mm512_log2_pd(A);
}

__m512d test_mm512_log_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log8(<8 x double> %{{.*}})
  return _mm512_log_pd(A);
}

__m512d test_mm512_pow_pd(__m512d A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_pow_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_pow8(<8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_pow_pd(A, B);
}

__m512d test_mm512_sinh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_sinh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sinh8(<8 x double> %{{.*}})
  return _mm512_sinh_pd(A);
}

__m512d test_mm512_sin_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_sin_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sin8(<8 x double> %{{.*}})
  return _mm512_sin_pd(A);
}

__m512d test_mm512_tanh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_tanh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tanh8(<8 x double> %{{.*}})
  return _mm512_tanh_pd(A);
}

__m512d test_mm512_tan_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_tan_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tan8(<8 x double> %{{.*}})
  return _mm512_tan_pd(A);
}




__m512i test_mm512_rem_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi32
  // CHECK-AVX512F: call svml_cc <16 x i32> @__svml_irem16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_rem_epi32(A, B);
}

__m512i test_mm512_div_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi32
  // CHECK-AVX512F: call svml_cc <16 x i32> @__svml_idiv16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_div_epi32(A, B);
}

__m512i test_mm512_rem_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu32
  // CHECK-AVX512F: call svml_cc <16 x i32> @__svml_urem16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_rem_epu32(A, B);
}

__m512i test_mm512_div_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu32
  // CHECK-AVX512F: call svml_cc <16 x i32> @__svml_udiv16(<16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_div_epu32(A, B);
}
#endif // __AVX512F__
