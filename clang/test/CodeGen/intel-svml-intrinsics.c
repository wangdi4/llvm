// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +sse2 -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-NOSSE41
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +sse4.1 -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-SSE41
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx2 -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1,CHECK-AVX2
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-linux-gnu -target-feature +avx512f -emit-llvm -opaque-pointers -o - -Wall -Werror | FileCheck %s --check-prefixes=CHECK,CHECK-AVX1,CHECK-AVX2,CHECK-AVX512F

#include <immintrin.h>

// SSE1 FP
__m128 test_mm_cbrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cbrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_cbrtf4(<4 x float> %{{.*}})
  return _mm_cbrt_ps(A);
}

__m128 test_mm_cexp_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cexp_ps
  // CHECK: call svml_cc <4 x float> @__svml_cexpf2(<4 x float> %{{.*}})
  return _mm_cexp_ps(A);
}

__m128 test_mm_clog_ps(__m128 A) {
  // CHECK-LABEL: test_mm_clog_ps
  // CHECK: call svml_cc <4 x float> @__svml_clogf2(<4 x float> %{{.*}})
  return _mm_clog_ps(A);
}

__m128 test_mm_csqrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_csqrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_csqrtf2(<4 x float> %{{.*}})
  return _mm_csqrt_ps(A);
}

__m128 test_mm_exp_ps(__m128 A) {
  // CHECK-LABEL: test_mm_exp_ps
  // CHECK: call svml_cc <4 x float> @__svml_expf4(<4 x float> %{{.*}})
  return _mm_exp_ps(A);
}

__m128 test_mm_exp10_ps(__m128 A) {
  // CHECK-LABEL: test_mm_exp10_ps
  // CHECK: call svml_cc <4 x float> @__svml_exp10f4(<4 x float> %{{.*}})
  return _mm_exp10_ps(A);
}

__m128 test_mm_exp2_ps(__m128 A) {
  // CHECK-LABEL: test_mm_exp2_ps
  // CHECK: call svml_cc <4 x float> @__svml_exp2f4(<4 x float> %{{.*}})
  return _mm_exp2_ps(A);
}

__m128 test_mm_expm1_ps(__m128 A) {
  // CHECK-LABEL: test_mm_expm1_ps
  // CHECK: call svml_cc <4 x float> @__svml_expm1f4(<4 x float> %{{.*}})
  return _mm_expm1_ps(A);
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

__m128 test_mm_log_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log_ps
  // CHECK: call svml_cc <4 x float> @__svml_logf4(<4 x float> %{{.*}})
  return _mm_log_ps(A);
}

__m128 test_mm_log10_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log10_ps
  // CHECK: call svml_cc <4 x float> @__svml_log10f4(<4 x float> %{{.*}})
  return _mm_log10_ps(A);
}

__m128 test_mm_log1p_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log1p_ps
  // CHECK: call svml_cc <4 x float> @__svml_log1pf4(<4 x float> %{{.*}})
  return _mm_log1p_ps(A);
}

__m128 test_mm_log2_ps(__m128 A) {
  // CHECK-LABEL: test_mm_log2_ps
  // CHECK: call svml_cc <4 x float> @__svml_log2f4(<4 x float> %{{.*}})
  return _mm_log2_ps(A);
}

__m128 test_mm_logb_ps(__m128 A) {
  // CHECK-LABEL: test_mm_logb_ps
  // CHECK: call svml_cc <4 x float> @__svml_logbf4(<4 x float> %{{.*}})
  return _mm_logb_ps(A);
}

__m128 test_mm_pow_ps(__m128 A, __m128 B) {
  // CHECK-LABEL: test_mm_pow_ps
  // CHECK: call svml_cc <4 x float> @__svml_powf4(<4 x float> %{{.*}}, <4 x float> %{{.*}})
  return _mm_pow_ps(A, B);
}

__m128 test_mm_svml_sqrt_ps(__m128 A) {
  // CHECK-LABEL: test_mm_svml_sqrt_ps
  // CHECK: call svml_cc <4 x float> @__svml_sqrtf4(<4 x float> %{{.*}})
  return _mm_svml_sqrt_ps(A);
}

__m128 test_mm_acos_ps(__m128 A) {
  // CHECK-LABEL: test_mm_acos_ps
  // CHECK: call svml_cc <4 x float> @__svml_acosf4(<4 x float> %{{.*}})
  return _mm_acos_ps(A);
}

__m128 test_mm_acosh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_acosh_ps
  // CHECK: call svml_cc <4 x float> @__svml_acoshf4(<4 x float> %{{.*}})
  return _mm_acosh_ps(A);
}

__m128 test_mm_asin_ps(__m128 A) {
  // CHECK-LABEL: test_mm_asin_ps
  // CHECK: call svml_cc <4 x float> @__svml_asinf4(<4 x float> %{{.*}})
  return _mm_asin_ps(A);
}

__m128 test_mm_asinh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_asinh_ps
  // CHECK: call svml_cc <4 x float> @__svml_asinhf4(<4 x float> %{{.*}})
  return _mm_asinh_ps(A);
}

__m128 test_mm_atan_ps(__m128 A) {
  // CHECK-LABEL: test_mm_atan_ps
  // CHECK: call svml_cc <4 x float> @__svml_atanf4(<4 x float> %{{.*}})
  return _mm_atan_ps(A);
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

__m128 test_mm_cos_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cos_ps
  // CHECK: call svml_cc <4 x float> @__svml_cosf4(<4 x float> %{{.*}})
  return _mm_cos_ps(A);
}

__m128 test_mm_cosd_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cosd_ps
  // CHECK: call svml_cc <4 x float> @__svml_cosdf4(<4 x float> %{{.*}})
  return _mm_cosd_ps(A);
}

__m128 test_mm_cosh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cosh_ps
  // CHECK: call svml_cc <4 x float> @__svml_coshf4(<4 x float> %{{.*}})
  return _mm_cosh_ps(A);
}

__m128 test_mm_hypot_ps(__m128 A, __m128 B) {
  // CHECK-LABEL: test_mm_hypot_ps
  // CHECK: call svml_cc <4 x float> @__svml_hypotf4(<4 x float> %{{.*}}, <4 x float> %{{.*}})
  return _mm_hypot_ps(A, B);
}

__m128 test_mm_sin_ps(__m128 A) {
  // CHECK-LABEL: test_mm_sin_ps
  // CHECK: call svml_cc <4 x float> @__svml_sinf4(<4 x float> %{{.*}})
  return _mm_sin_ps(A);
}

__m128 test_mm_sincos_ps(__m128 *A, __m128 B) {
  // CHECK-LABEL: test_mm_sincos_ps
  // CHECK: [[RESULT:%.*]] = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float> %{{.*}})
  // CHECK: [[COS:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 1
  // CHECK: store <4 x float> [[COS]], ptr %{{.*}}
  // CHECK: [[SIN:%.*]] = extractvalue { <4 x float>, <4 x float> } [[RESULT]], 0
  // CHECK: ret <4 x float> [[SIN]]
  return _mm_sincos_ps(A, B);
}

__m128 test_mm_sind_ps(__m128 A) {
  // CHECK-LABEL: test_mm_sind_ps
  // CHECK: call svml_cc <4 x float> @__svml_sindf4(<4 x float> %{{.*}})
  return _mm_sind_ps(A);
}

__m128 test_mm_sinh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_sinh_ps
  // CHECK: call svml_cc <4 x float> @__svml_sinhf4(<4 x float> %{{.*}})
  return _mm_sinh_ps(A);
}

__m128 test_mm_tan_ps(__m128 A) {
  // CHECK-LABEL: test_mm_tan_ps
  // CHECK: call svml_cc <4 x float> @__svml_tanf4(<4 x float> %{{.*}})
  return _mm_tan_ps(A);
}

__m128 test_mm_tand_ps(__m128 A) {
  // CHECK-LABEL: test_mm_tand_ps
  // CHECK: call svml_cc <4 x float> @__svml_tandf4(<4 x float> %{{.*}})
  return _mm_tand_ps(A);
}

__m128 test_mm_tanh_ps(__m128 A) {
  // CHECK-LABEL: test_mm_tanh_ps
  // CHECK: call svml_cc <4 x float> @__svml_tanhf4(<4 x float> %{{.*}})
  return _mm_tanh_ps(A);
}

__m128 test_mm_cdfnorm_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cdfnorm_ps
  // CHECK: call svml_cc <4 x float> @__svml_cdfnormf4(<4 x float> %{{.*}})
  return _mm_cdfnorm_ps(A);
}

__m128 test_mm_cdfnorminv_ps(__m128 A) {
  // CHECK-LABEL: test_mm_cdfnorminv_ps
  // CHECK: call svml_cc <4 x float> @__svml_cdfnorminvf4(<4 x float> %{{.*}})
  return _mm_cdfnorminv_ps(A);
}

__m128 test_mm_erf_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erf_ps
  // CHECK: call svml_cc <4 x float> @__svml_erff4(<4 x float> %{{.*}})
  return _mm_erf_ps(A);
}

__m128 test_mm_erfc_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erfc_ps
  // CHECK: call svml_cc <4 x float> @__svml_erfcf4(<4 x float> %{{.*}})
  return _mm_erfc_ps(A);
}

__m128 test_mm_erfcinv_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erfcinv_ps
  // CHECK: call svml_cc <4 x float> @__svml_erfcinvf4(<4 x float> %{{.*}})
  return _mm_erfcinv_ps(A);
}

__m128 test_mm_erfinv_ps(__m128 A) {
  // CHECK-LABEL: test_mm_erfinv_ps
  // CHECK: call svml_cc <4 x float> @__svml_erfinvf4(<4 x float> %{{.*}})
  return _mm_erfinv_ps(A);
}

__m128 test_mm_svml_round_ps(__m128 A) {
  // CHECK-LABEL: test_mm_svml_round_ps
  // CHECK: call svml_cc <4 x float> @__svml_roundf4(<4 x float> %{{.*}})
  return _mm_svml_round_ps(A);
}

__m128 test_mm_svml_ceil_ps(__m128 A) {
  // CHECK-LABEL: test_mm_svml_ceil_ps
  // CHECK-NOSSE41: call svml_cc <4 x float> @__svml_ceilf4(<4 x float> %{{.*}})
  // CHECK-SSE41: call <4 x float> @llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 2)
  return _mm_svml_ceil_ps(A);
}

__m128 test_mm_svml_floor_ps(__m128 A) {
  // CHECK-LABEL: test_mm_svml_floor_ps
  // CHECK-NOSSE41: call svml_cc <4 x float> @__svml_floorf4(<4 x float> %{{.*}})
  // CHECK-SSE41: call <4 x float> @llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 1)
  return _mm_svml_floor_ps(A);
}

__m128 test_mm_trunc_ps(__m128 A) {
  // CHECK-LABEL: test_mm_trunc_ps
  // CHECK-NOSSE41: call svml_cc <4 x float> @__svml_truncf4(<4 x float> %{{.*}})
  // CHECK-SSE41: call <4 x float> @llvm.x86.sse41.round.ps(<4 x float> %{{.*}}, i32 3)
  return _mm_trunc_ps(A);
}

// SSE2 FP
__m128d test_mm_erf_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erf_pd
  // CHECK: call svml_cc <2 x double> @__svml_erf2(<2 x double> %{{.*}})
  return _mm_erf_pd(A);
}

__m128d test_mm_cbrt_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cbrt_pd
  // CHECK: call svml_cc <2 x double> @__svml_cbrt2(<2 x double> %{{.*}})
  return _mm_cbrt_pd(A);
}

__m128d test_mm_exp_pd(__m128d A) {
  // CHECK-LABEL: test_mm_exp_pd
  // CHECK: call svml_cc <2 x double> @__svml_exp2(<2 x double> %{{.*}})
  return _mm_exp_pd(A);
}

__m128d test_mm_exp10_pd(__m128d A) {
  // CHECK-LABEL: test_mm_exp10_pd
  // CHECK: call svml_cc <2 x double> @__svml_exp102(<2 x double> %{{.*}})
  return _mm_exp10_pd(A);
}

__m128d test_mm_exp2_pd(__m128d A) {
  // CHECK-LABEL: test_mm_exp2_pd
  // CHECK: call svml_cc <2 x double> @__svml_exp22(<2 x double> %{{.*}})
  return _mm_exp2_pd(A);
}

__m128d test_mm_expm1_pd(__m128d A) {
  // CHECK-LABEL: test_mm_expm1_pd
  // CHECK: call svml_cc <2 x double> @__svml_expm12(<2 x double> %{{.*}})
  return _mm_expm1_pd(A);
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

__m128d test_mm_log_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log_pd
  // CHECK: call svml_cc <2 x double> @__svml_log2(<2 x double> %{{.*}})
  return _mm_log_pd(A);
}

__m128d test_mm_log10_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log10_pd
  // CHECK: call svml_cc <2 x double> @__svml_log102(<2 x double> %{{.*}})
  return _mm_log10_pd(A);
}

__m128d test_mm_log1p_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log1p_pd
  // CHECK: call svml_cc <2 x double> @__svml_log1p2(<2 x double> %{{.*}})
  return _mm_log1p_pd(A);
}

__m128d test_mm_log2_pd(__m128d A) {
  // CHECK-LABEL: test_mm_log2_pd
  // CHECK: call svml_cc <2 x double> @__svml_log22(<2 x double> %{{.*}})
  return _mm_log2_pd(A);
}

__m128d test_mm_logb_pd(__m128d A) {
  // CHECK-LABEL: test_mm_logb_pd
  // CHECK: call svml_cc <2 x double> @__svml_logb2(<2 x double> %{{.*}})
  return _mm_logb_pd(A);
}

__m128d test_mm_pow_pd(__m128d A, __m128d B) {
  // CHECK-LABEL: test_mm_pow_pd
  // CHECK: call svml_cc <2 x double> @__svml_pow2(<2 x double> %{{.*}}, <2 x double> %{{.*}})
  return _mm_pow_pd(A, B);
}

__m128d test_mm_svml_sqrt_pd(__m128d A) {
  // CHECK-LABEL: test_mm_svml_sqrt_pd
  // CHECK: call svml_cc <2 x double> @__svml_sqrt2(<2 x double> %{{.*}})
  return _mm_svml_sqrt_pd(A);
}

__m128d test_mm_acos_pd(__m128d A) {
  // CHECK-LABEL: test_mm_acos_pd
  // CHECK: call svml_cc <2 x double> @__svml_acos2(<2 x double> %{{.*}})
  return _mm_acos_pd(A);
}

__m128d test_mm_acosh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_acosh_pd
  // CHECK: call svml_cc <2 x double> @__svml_acosh2(<2 x double> %{{.*}})
  return _mm_acosh_pd(A);
}

__m128d test_mm_asin_pd(__m128d A) {
  // CHECK-LABEL: test_mm_asin_pd
  // CHECK: call svml_cc <2 x double> @__svml_asin2(<2 x double> %{{.*}})
  return _mm_asin_pd(A);
}

__m128d test_mm_asinh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_asinh_pd
  // CHECK: call svml_cc <2 x double> @__svml_asinh2(<2 x double> %{{.*}})
  return _mm_asinh_pd(A);
}

__m128d test_mm_atan_pd(__m128d A) {
  // CHECK-LABEL: test_mm_atan_pd
  // CHECK: call svml_cc <2 x double> @__svml_atan2(<2 x double> %{{.*}})
  return _mm_atan_pd(A);
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

__m128d test_mm_cos_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cos_pd
  // CHECK: call svml_cc <2 x double> @__svml_cos2(<2 x double> %{{.*}})
  return _mm_cos_pd(A);
}

__m128d test_mm_cosd_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cosd_pd
  // CHECK: call svml_cc <2 x double> @__svml_cosd2(<2 x double> %{{.*}})
  return _mm_cosd_pd(A);
}

__m128d test_mm_cosh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cosh_pd
  // CHECK: call svml_cc <2 x double> @__svml_cosh2(<2 x double> %{{.*}})
  return _mm_cosh_pd(A);
}

__m128d test_mm_hypot_pd(__m128d A, __m128d B) {
  // CHECK-LABEL: test_mm_hypot_pd
  // CHECK: call svml_cc <2 x double> @__svml_hypot2(<2 x double> %{{.*}}, <2 x double> %{{.*}})
  return _mm_hypot_pd(A, B);
}

__m128d test_mm_sin_pd(__m128d A) {
  // CHECK-LABEL: test_mm_sin_pd
  // CHECK: call svml_cc <2 x double> @__svml_sin2(<2 x double> %{{.*}})
  return _mm_sin_pd(A);
}

__m128d test_mm_sincos_pd(__m128d *A, __m128d B) {
  // CHECK-LABEL: test_mm_sincos_pd
  // CHECK: [[RESULT:%.*]] = call svml_cc { <2 x double>, <2 x double> } @__svml_sincos2(<2 x double> %{{.*}})
  // CHECK: [[COS:%.*]] = extractvalue { <2 x double>, <2 x double> } [[RESULT]], 1
  // CHECK: store <2 x double> [[COS]], ptr %{{.*}}
  // CHECK: [[SIN:%.*]] = extractvalue { <2 x double>, <2 x double> } [[RESULT]], 0
  // CHECK: ret <2 x double> [[SIN]]
  return _mm_sincos_pd(A, B);
}

__m128d test_mm_sind_pd(__m128d A) {
  // CHECK-LABEL: test_mm_sind_pd
  // CHECK: call svml_cc <2 x double> @__svml_sind2(<2 x double> %{{.*}})
  return _mm_sind_pd(A);
}

__m128d test_mm_sinh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_sinh_pd
  // CHECK: call svml_cc <2 x double> @__svml_sinh2(<2 x double> %{{.*}})
  return _mm_sinh_pd(A);
}

__m128d test_mm_tan_pd(__m128d A) {
  // CHECK-LABEL: test_mm_tan_pd
  // CHECK: call svml_cc <2 x double> @__svml_tan2(<2 x double> %{{.*}})
  return _mm_tan_pd(A);
}

__m128d test_mm_tand_pd(__m128d A) {
  // CHECK-LABEL: test_mm_tand_pd
  // CHECK: call svml_cc <2 x double> @__svml_tand2(<2 x double> %{{.*}})
  return _mm_tand_pd(A);
}

__m128d test_mm_tanh_pd(__m128d A) {
  // CHECK-LABEL: test_mm_tanh_pd
  // CHECK: call svml_cc <2 x double> @__svml_tanh2(<2 x double> %{{.*}})
  return _mm_tanh_pd(A);
}

__m128d test_mm_cdfnorm_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cdfnorm_pd
  // CHECK: call svml_cc <2 x double> @__svml_cdfnorm2(<2 x double> %{{.*}})
  return _mm_cdfnorm_pd(A);
}

__m128d test_mm_cdfnorminv_pd(__m128d A) {
  // CHECK-LABEL: test_mm_cdfnorminv_pd
  // CHECK: call svml_cc <2 x double> @__svml_cdfnorminv2(<2 x double> %{{.*}})
  return _mm_cdfnorminv_pd(A);
}

__m128d test_mm_erfc_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erfc_pd
  // CHECK: call svml_cc <2 x double> @__svml_erfc2(<2 x double> %{{.*}})
  return _mm_erfc_pd(A);
}

__m128d test_mm_erfcinv_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erfcinv_pd
  // CHECK: call svml_cc <2 x double> @__svml_erfcinv2(<2 x double> %{{.*}})
  return _mm_erfcinv_pd(A);
}

__m128d test_mm_erfinv_pd(__m128d A) {
  // CHECK-LABEL: test_mm_erfinv_pd
  // CHECK: call svml_cc <2 x double> @__svml_erfinv2(<2 x double> %{{.*}})
  return _mm_erfinv_pd(A);
}

__m128d test_mm_svml_round_pd(__m128d A) {
  // CHECK-LABEL: test_mm_svml_round_pd
  // CHECK: call svml_cc <2 x double> @__svml_round2(<2 x double> %{{.*}})
  return _mm_svml_round_pd(A);
}

__m128d test_mm_svml_ceil_pd(__m128d A) {
  // CHECK-LABEL: test_mm_svml_ceil_pd
  // CHECK-NOSSE41: call svml_cc <2 x double> @__svml_ceil2(<2 x double> %{{.*}})
  // CHECK-SSE41: call <2 x double> @llvm.x86.sse41.round.pd(<2 x double> %{{.*}}, i32 2)
  return _mm_svml_ceil_pd(A);
}

__m128d test_mm_svml_floor_pd(__m128d A) {
  // CHECK-LABEL: test_mm_svml_floor_pd
  // CHECK-NOSSE41: call svml_cc <2 x double> @__svml_floor2(<2 x double> %{{.*}})
  // CHECK-SSE41: call <2 x double> @llvm.x86.sse41.round.pd(<2 x double> %{{.*}}, i32 1)
  return _mm_svml_floor_pd(A);
}

__m128d test_mm_trunc_pd(__m128d A) {
  // CHECK-LABEL: test_mm_trunc_pd
  // CHECK-NOSSE41: call svml_cc <2 x double> @__svml_trunc2(<2 x double> %{{.*}})
  // CHECK-SSE41: call <2 x double> @llvm.x86.sse41.round.pd(<2 x double> %{{.*}}, i32 3)
  return _mm_trunc_pd(A);
}

// SSE2 Int
__m128i test_mm_div_epi8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epi8
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[RESULT:%.*]] = call svml_cc <16 x i8> @__svml_i8div16(<16 x i8> [[DIVIDEND]], <16 x i8> [[DIVISOR]])
  // CHECK: bitcast <16 x i8> [[RESULT]] to <2 x i64>
  return _mm_div_epi8(A, B);
}

__m128i test_mm_div_epu8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epu8
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[RESULT:%.*]] = call svml_cc <16 x i8> @__svml_u8div16(<16 x i8> [[DIVIDEND]], <16 x i8> [[DIVISOR]])
  // CHECK: bitcast <16 x i8> [[RESULT]] to <2 x i64>
  return _mm_div_epu8(A, B);
}

__m128i test_mm_div_epi16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epi16
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[RESULT:%.*]] = call svml_cc <8 x i16> @__svml_i16div8(<8 x i16> [[DIVIDEND]], <8 x i16> [[DIVISOR]])
  // CHECK: bitcast <8 x i16> [[RESULT]] to <2 x i64>
  return _mm_div_epi16(A, B);
}

__m128i test_mm_div_epu16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epu16
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[RESULT:%.*]] = call svml_cc <8 x i16> @__svml_u16div8(<8 x i16> [[DIVIDEND]], <8 x i16> [[DIVISOR]])
  // CHECK: bitcast <8 x i16> [[RESULT]] to <2 x i64>
  return _mm_div_epu16(A, B);
}

__m128i test_mm_div_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_div_epi32(A, B);
}

__m128i test_mm_div_epu32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epu32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_udiv4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_div_epu32(A, B);
}

__m128i test_mm_div_epi64(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epi64
  // CHECK: call svml_cc <2 x i64> @__svml_i64div2(<2 x i64> {{.*}}, <2 x i64> {{.*}})
  return _mm_div_epi64(A, B);
}

__m128i test_mm_div_epu64(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_div_epu64
  // CHECK: call svml_cc <2 x i64> @__svml_u64div2(<2 x i64> {{.*}}, <2 x i64> {{.*}})
  return _mm_div_epu64(A, B);
}

__m128i test_mm_idiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_idiv_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_idiv4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_idiv_epi32(A, B);
}

__m128i test_mm_idivrem_epi32(__m128i *A, __m128i B, __m128i C) {
  // CHECK-LABEL: test_mm_idivrem_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc { <4 x i32>, <4 x i32> } @__svml_idivrem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: [[REMAINDER:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 1
  // CHECK: [[REMAINDER_CAST:%.*]] = bitcast <4 x i32> [[REMAINDER]] to <2 x i64>
  // CHECK: store <2 x i64> [[REMAINDER_CAST]], ptr %{{.*}}
  // CHECK: [[QUOTIENT:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 0
  // CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <4 x i32> [[QUOTIENT]] to <2 x i64>
  // CHECK: ret <2 x i64> [[QUOTIENT_CAST]]
  return _mm_idivrem_epi32(A, B, C);
}

__m128i test_mm_irem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_irem_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_irem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_irem_epi32(A, B);
}

__m128i test_mm_rem_epi8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epi8
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[RESULT:%.*]] = call svml_cc <16 x i8> @__svml_i8rem16(<16 x i8> [[DIVIDEND]], <16 x i8> [[DIVISOR]])
  // CHECK: bitcast <16 x i8> [[RESULT]] to <2 x i64>
  return _mm_rem_epi8(A, B);
}

__m128i test_mm_rem_epu8(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epu8
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <16 x i8>
  // CHECK: [[RESULT:%.*]] = call svml_cc <16 x i8> @__svml_u8rem16(<16 x i8> [[DIVIDEND]], <16 x i8> [[DIVISOR]])
  // CHECK: bitcast <16 x i8> [[RESULT]] to <2 x i64>
  return _mm_rem_epu8(A, B);
}

__m128i test_mm_rem_epi16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epi16
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[RESULT:%.*]] = call svml_cc <8 x i16> @__svml_i16rem8(<8 x i16> [[DIVIDEND]], <8 x i16> [[DIVISOR]])
  // CHECK: bitcast <8 x i16> [[RESULT]] to <2 x i64>
  return _mm_rem_epi16(A, B);
}

__m128i test_mm_rem_epu16(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epu16
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK: [[RESULT:%.*]] = call svml_cc <8 x i16> @__svml_u16rem8(<8 x i16> [[DIVIDEND]], <8 x i16> [[DIVISOR]])
  // CHECK: bitcast <8 x i16> [[RESULT]] to <2 x i64>
  return _mm_rem_epu16(A, B);
}

__m128i test_mm_rem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_irem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_rem_epi32(A, B);
}

__m128i test_mm_rem_epu32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epu32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_urem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_rem_epu32(A, B);
}

__m128i test_mm_rem_epi64(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epi64
  // CHECK: call svml_cc <2 x i64> @__svml_i64rem2(<2 x i64> {{.*}}, <2 x i64> {{.*}})
  return _mm_rem_epi64(A, B);
}

__m128i test_mm_rem_epu64(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_rem_epu64
  // CHECK: call svml_cc <2 x i64> @__svml_u64rem2(<2 x i64> {{.*}}, <2 x i64> {{.*}})
  return _mm_rem_epu64(A, B);
}

__m128i test_mm_udiv_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_udiv_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_udiv4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_udiv_epi32(A, B);
}

__m128i test_mm_udivrem_epi32(__m128i *A, __m128i B, __m128i C) {
  // CHECK-LABEL: test_mm_udivrem_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc { <4 x i32>, <4 x i32> } @__svml_udivrem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: [[REMAINDER:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 1
  // CHECK: [[REMAINDER_CAST:%.*]] = bitcast <4 x i32> [[REMAINDER]] to <2 x i64>
  // CHECK: store <2 x i64> [[REMAINDER_CAST]], ptr %{{.*}}
  // CHECK: [[QUOTIENT:%.*]] = extractvalue { <4 x i32>, <4 x i32> } [[RESULT]], 0
  // CHECK: [[QUOTIENT_CAST:%.*]] = bitcast <4 x i32> [[QUOTIENT]] to <2 x i64>
  // CHECK: ret <2 x i64> [[QUOTIENT_CAST]]
  return _mm_udivrem_epi32(A, B, C);
}

__m128i test_mm_urem_epi32(__m128i A, __m128i B) {
  // CHECK-LABEL: test_mm_urem_epi32
  // CHECK: [[DIVIDEND:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[DIVISOR:%.*]] = bitcast <2 x i64> %{{.*}} to <4 x i32>
  // CHECK: [[RESULT:%.*]] = call svml_cc <4 x i32> @__svml_urem4(<4 x i32> [[DIVIDEND]], <4 x i32> [[DIVISOR]])
  // CHECK: bitcast <4 x i32> [[RESULT]] to <2 x i64>
  return _mm_urem_epi32(A, B);
}

// AVX single precision
#ifdef __AVX__
__m256 test_mm256_cbrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cbrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cbrtf8(<8 x float> %{{.*}})
  return _mm256_cbrt_ps(A);
}

__m256 test_mm256_cexp_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cexp_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cexpf4(<8 x float> %{{.*}})
  return _mm256_cexp_ps(A);
}

__m256 test_mm256_clog_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_clog_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_clogf4(<8 x float> %{{.*}})
  return _mm256_clog_ps(A);
}

__m256 test_mm256_csqrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_csqrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_csqrtf4(<8 x float> %{{.*}})
  return _mm256_csqrt_ps(A);
}

__m256 test_mm256_exp_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_exp_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_expf8(<8 x float> %{{.*}})
  return _mm256_exp_ps(A);
}

__m256 test_mm256_exp10_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_exp10_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_exp10f8(<8 x float> %{{.*}})
  return _mm256_exp10_ps(A);
}

__m256 test_mm256_exp2_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_exp2_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_exp2f8(<8 x float> %{{.*}})
  return _mm256_exp2_ps(A);
}

__m256 test_mm256_expm1_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_expm1_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_expm1f8(<8 x float> %{{.*}})
  return _mm256_expm1_ps(A);
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

__m256 test_mm256_log_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_logf8(<8 x float> %{{.*}})
  return _mm256_log_ps(A);
}

__m256 test_mm256_log10_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log10_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_log10f8(<8 x float> %{{.*}})
  return _mm256_log10_ps(A);
}

__m256 test_mm256_log1p_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log1p_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_log1pf8(<8 x float> %{{.*}})
  return _mm256_log1p_ps(A);
}

__m256 test_mm256_log2_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_log2_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_log2f8(<8 x float> %{{.*}})
  return _mm256_log2_ps(A);
}

__m256 test_mm256_logb_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_logb_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_logbf8(<8 x float> %{{.*}})
  return _mm256_logb_ps(A);
}

__m256 test_mm256_pow_ps(__m256 A, __m256 B) {
  // CHECK-AVX1-LABEL: test_mm256_pow_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_powf8(<8 x float> %{{.*}}, <8 x float> %{{.*}})
  return _mm256_pow_ps(A, B);
}

__m256 test_mm256_svml_sqrt_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_sqrt_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sqrtf8(<8 x float> %{{.*}})
  return _mm256_svml_sqrt_ps(A);
}

__m256 test_mm256_acos_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_acos_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_acosf8(<8 x float> %{{.*}})
  return _mm256_acos_ps(A);
}

__m256 test_mm256_acosh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_acosh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_acoshf8(<8 x float> %{{.*}})
  return _mm256_acosh_ps(A);
}

__m256 test_mm256_asin_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_asin_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_asinf8(<8 x float> %{{.*}})
  return _mm256_asin_ps(A);
}

__m256 test_mm256_asinh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_asinh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_asinhf8(<8 x float> %{{.*}})
  return _mm256_asinh_ps(A);
}

__m256 test_mm256_atan_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_atan_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_atanf8(<8 x float> %{{.*}})
  return _mm256_atan_ps(A);
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

__m256 test_mm256_cos_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cos_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cosf8(<8 x float> %{{.*}})
  return _mm256_cos_ps(A);
}

__m256 test_mm256_cosd_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cosd_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cosdf8(<8 x float> %{{.*}})
  return _mm256_cosd_ps(A);
}

__m256 test_mm256_cosh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cosh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_coshf8(<8 x float> %{{.*}})
  return _mm256_cosh_ps(A);
}

__m256 test_mm256_hypot_ps(__m256 A, __m256 B) {
  // CHECK-AVX1-LABEL: test_mm256_hypot_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_hypotf8(<8 x float> %{{.*}}, <8 x float> %{{.*}})
  return _mm256_hypot_ps(A, B);
}

__m256 test_mm256_sin_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_sin_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sinf8(<8 x float> %{{.*}})
  return _mm256_sin_ps(A);
}

__m256 test_mm256_sincos_ps(__m256 *A, __m256 B) {
  // CHECK-AVX1-LABEL: test_mm256_sincos_ps
  // CHECK-AVX1: [[RESULT:%.*]] = call svml_cc { <8 x float>, <8 x float> } @__svml_sincosf8(<8 x float> %{{.*}})
  // CHECK-AVX1: [[COS:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 1
  // CHECK-AVX1: store <8 x float> [[COS]], ptr %{{.*}}
  // CHECK-AVX1: [[SIN:%.*]] = extractvalue { <8 x float>, <8 x float> } [[RESULT]], 0
  // CHECK-AVX1: ret <8 x float> [[SIN]]
  return _mm256_sincos_ps(A, B);
}

__m256 test_mm256_sind_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_sind_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sindf8(<8 x float> %{{.*}})
  return _mm256_sind_ps(A);
}

__m256 test_mm256_sinh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_sinh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_sinhf8(<8 x float> %{{.*}})
  return _mm256_sinh_ps(A);
}

__m256 test_mm256_tan_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_tan_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_tanf8(<8 x float> %{{.*}})
  return _mm256_tan_ps(A);
}

__m256 test_mm256_tand_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_tand_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_tandf8(<8 x float> %{{.*}})
  return _mm256_tand_ps(A);
}

__m256 test_mm256_tanh_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_tanh_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_tanhf8(<8 x float> %{{.*}})
  return _mm256_tanh_ps(A);
}

__m256 test_mm256_cdfnorm_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cdfnorm_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cdfnormf8(<8 x float> %{{.*}})
  return _mm256_cdfnorm_ps(A);
}

__m256 test_mm256_cdfnorminv_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_cdfnorminv_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_cdfnorminvf8(<8 x float> %{{.*}})
  return _mm256_cdfnorminv_ps(A);
}

__m256 test_mm256_erf_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erf_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erff8(<8 x float> %{{.*}})
  return _mm256_erf_ps(A);
}

__m256 test_mm256_erfc_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erfc_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erfcf8(<8 x float> %{{.*}})
  return _mm256_erfc_ps(A);
}

__m256 test_mm256_erfcinv_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erfcinv_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erfcinvf8(<8 x float> %{{.*}})
  return _mm256_erfcinv_ps(A);
}

__m256 test_mm256_erfinv_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_erfinv_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_erfinvf8(<8 x float> %{{.*}})
  return _mm256_erfinv_ps(A);
}

__m256 test_mm256_svml_round_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_round_ps
  // CHECK-AVX1: call svml_cc <8 x float> @__svml_roundf8(<8 x float> %{{.*}})
  return _mm256_svml_round_ps(A);
}

__m256 test_mm256_svml_ceil_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_ceil_ps
  // CHECK-AVX1: call <8 x float> @llvm.x86.avx.round.ps.256(<8 x float> %{{.*}}, i32 2)
  return _mm256_svml_ceil_ps(A);
}

__m256 test_mm256_svml_floor_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_floor_ps
  // CHECK-AVX1: call <8 x float> @llvm.x86.avx.round.ps.256(<8 x float> %{{.*}}, i32 1)
  return _mm256_svml_floor_ps(A);
}

__m256 test_mm256_trunc_ps(__m256 A) {
  // CHECK-AVX1-LABEL: test_mm256_trunc_ps
  // CHECK-AVX1: call <8 x float> @llvm.x86.avx.round.ps.256(<8 x float> %{{.*}}, i32 3)
  return _mm256_trunc_ps(A);
}

// AVX double precision
__m256d test_mm256_cbrt_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cbrt_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cbrt4(<4 x double> %{{.*}})
  return _mm256_cbrt_pd(A);
}

__m256d test_mm256_exp_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_exp_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_exp4(<4 x double> %{{.*}})
  return _mm256_exp_pd(A);
}

__m256d test_mm256_exp10_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_exp10_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_exp104(<4 x double> %{{.*}})
  return _mm256_exp10_pd(A);
}

__m256d test_mm256_exp2_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_exp2_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_exp24(<4 x double> %{{.*}})
  return _mm256_exp2_pd(A);
}

__m256d test_mm256_expm1_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_expm1_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_expm14(<4 x double> %{{.*}})
  return _mm256_expm1_pd(A);
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

__m256d test_mm256_log_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log4(<4 x double> %{{.*}})
  return _mm256_log_pd(A);
}

__m256d test_mm256_log10_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log10_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log104(<4 x double> %{{.*}})
  return _mm256_log10_pd(A);
}

__m256d test_mm256_log1p_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log1p_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log1p4(<4 x double> %{{.*}})
  return _mm256_log1p_pd(A);
}

__m256d test_mm256_log2_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_log2_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_log24(<4 x double> %{{.*}})
  return _mm256_log2_pd(A);
}

__m256d test_mm256_logb_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_logb_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_logb4(<4 x double> %{{.*}})
  return _mm256_logb_pd(A);
}

__m256d test_mm256_pow_pd(__m256d A, __m256d B) {
  // CHECK-AVX1-LABEL: test_mm256_pow_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_pow4(<4 x double> %{{.*}}, <4 x double> %{{.*}})
  return _mm256_pow_pd(A, B);
}

__m256d test_mm256_svml_sqrt_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_sqrt_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sqrt4(<4 x double> %{{.*}})
  return _mm256_svml_sqrt_pd(A);
}

__m256d test_mm256_acos_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_acos_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_acos4(<4 x double> %{{.*}})
  return _mm256_acos_pd(A);
}

__m256d test_mm256_acosh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_acosh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_acosh4(<4 x double> %{{.*}})
  return _mm256_acosh_pd(A);
}

__m256d test_mm256_asin_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_asin_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_asin4(<4 x double> %{{.*}})
  return _mm256_asin_pd(A);
}

__m256d test_mm256_asinh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_asinh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_asinh4(<4 x double> %{{.*}})
  return _mm256_asinh_pd(A);
}

__m256d test_mm256_atan_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_atan_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_atan4(<4 x double> %{{.*}})
  return _mm256_atan_pd(A);
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

__m256d test_mm256_cos_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cos_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cos4(<4 x double> %{{.*}})
  return _mm256_cos_pd(A);
}

__m256d test_mm256_cosd_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cosd_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cosd4(<4 x double> %{{.*}})
  return _mm256_cosd_pd(A);
}

__m256d test_mm256_cosh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cosh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cosh4(<4 x double> %{{.*}})
  return _mm256_cosh_pd(A);
}

__m256d test_mm256_hypot_pd(__m256d A, __m256d B) {
  // CHECK-AVX1-LABEL: test_mm256_hypot_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_hypot4(<4 x double> %{{.*}}, <4 x double> %{{.*}})
  return _mm256_hypot_pd(A, B);
}

__m256d test_mm256_sin_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_sin_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sin4(<4 x double> %{{.*}})
  return _mm256_sin_pd(A);
}

__m256d test_mm256_sincos_pd(__m256d *A, __m256d B) {
  // CHECK-AVX1-LABEL: test_mm256_sincos_pd
  // CHECK-AVX1: [[RESULT:%.*]] = call svml_cc { <4 x double>, <4 x double> } @__svml_sincos4(<4 x double> %{{.*}})
  // CHECK-AVX1: [[COS:%.*]] = extractvalue { <4 x double>, <4 x double> } [[RESULT]], 1
  // CHECK-AVX1: store <4 x double> [[COS]], ptr %{{.*}}
  // CHECK-AVX1: [[SIN:%.*]] = extractvalue { <4 x double>, <4 x double> } [[RESULT]], 0
  // CHECK-AVX1: ret <4 x double> [[SIN]]
  return _mm256_sincos_pd(A, B);
}

__m256d test_mm256_sind_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_sind_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sind4(<4 x double> %{{.*}})
  return _mm256_sind_pd(A);
}

__m256d test_mm256_sinh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_sinh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_sinh4(<4 x double> %{{.*}})
  return _mm256_sinh_pd(A);
}

__m256d test_mm256_tan_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_tan_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_tan4(<4 x double> %{{.*}})
  return _mm256_tan_pd(A);
}

__m256d test_mm256_tand_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_tand_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_tand4(<4 x double> %{{.*}})
  return _mm256_tand_pd(A);
}

__m256d test_mm256_tanh_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_tanh_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_tanh4(<4 x double> %{{.*}})
  return _mm256_tanh_pd(A);
}

__m256d test_mm256_cdfnorm_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cdfnorm_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cdfnorm4(<4 x double> %{{.*}})
  return _mm256_cdfnorm_pd(A);
}

__m256d test_mm256_cdfnorminv_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_cdfnorminv_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_cdfnorminv4(<4 x double> %{{.*}})
  return _mm256_cdfnorminv_pd(A);
}

__m256d test_mm256_erf_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erf_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erf4(<4 x double> %{{.*}})
  return _mm256_erf_pd(A);
}

__m256d test_mm256_erfc_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erfc_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erfc4(<4 x double> %{{.*}})
  return _mm256_erfc_pd(A);
}

__m256d test_mm256_erfcinv_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erfcinv_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erfcinv4(<4 x double> %{{.*}})
  return _mm256_erfcinv_pd(A);
}

__m256d test_mm256_erfinv_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_erfinv_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_erfinv4(<4 x double> %{{.*}})
  return _mm256_erfinv_pd(A);
}

__m256d test_mm256_svml_round_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_round_pd
  // CHECK-AVX1: call svml_cc <4 x double> @__svml_round4(<4 x double> %{{.*}})
  return _mm256_svml_round_pd(A);
}

__m256d test_mm256_svml_ceil_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_ceil_pd
  // CHECK-AVX1: call <4 x double> @llvm.x86.avx.round.pd.256(<4 x double> %{{.*}}, i32 2)
  return _mm256_svml_ceil_pd(A);
}

__m256d test_mm256_svml_floor_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_svml_floor_pd
  // CHECK-AVX1: call <4 x double> @llvm.x86.avx.round.pd.256(<4 x double> %{{.*}}, i32 1)
  return _mm256_svml_floor_pd(A);
}

__m256d test_mm256_trunc_pd(__m256d A) {
  // CHECK-AVX1-LABEL: test_mm256_trunc_pd
  // CHECK-AVX1: call <4 x double> @llvm.x86.avx.round.pd.256(<4 x double> %{{.*}}, i32 3)
  return _mm256_trunc_pd(A);
}
#endif // __AVX__

// AVX2 Int
#ifdef __AVX2__
__m256i test_mm256_div_epi8(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi8
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <32 x i8> @__svml_i8div32(<32 x i8> [[DIVIDEND]], <32 x i8> [[DIVISOR]])
  // CHECK-AVX2: bitcast <32 x i8> [[RESULT]] to <4 x i64>
  return _mm256_div_epi8(A, B);
}

__m256i test_mm256_div_epu8(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu8
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <32 x i8> @__svml_u8div32(<32 x i8> [[DIVIDEND]], <32 x i8> [[DIVISOR]])
  // CHECK-AVX2: bitcast <32 x i8> [[RESULT]] to <4 x i64>
  return _mm256_div_epu8(A, B);
}

__m256i test_mm256_div_epi16(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi16
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <16 x i16> @__svml_i16div16(<16 x i16> [[DIVIDEND]], <16 x i16> [[DIVISOR]])
  // CHECK-AVX2: bitcast <16 x i16> [[RESULT]] to <4 x i64>
  return _mm256_div_epi16(A, B);
}

__m256i test_mm256_div_epu16(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu16
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <16 x i16> @__svml_u16div16(<16 x i16> [[DIVIDEND]], <16 x i16> [[DIVISOR]])
  // CHECK-AVX2: bitcast <16 x i16> [[RESULT]] to <4 x i64>
  return _mm256_div_epu16(A, B);
}

__m256i test_mm256_div_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_div_epi32(A, B);
}

__m256i test_mm256_div_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_udiv8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_div_epu32(A, B);
}

__m256i test_mm256_div_epi64(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epi64
  // CHECK-AVX2: call svml_cc <4 x i64> @__svml_i64div4(<4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_div_epi64(A, B);
}

__m256i test_mm256_div_epu64(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_div_epu64
  // CHECK-AVX2: call svml_cc <4 x i64> @__svml_u64div4(<4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_div_epu64(A, B);
}

__m256i test_mm256_idiv_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_idiv_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_idiv8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_idiv_epi32(A, B);
}

__m256i test_mm256_idivrem_epi32(__m256i *A, __m256i B, __m256i C) {
  // CHECK-AVX2-LABEL: test_mm256_idivrem_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc { <8 x i32>, <8 x i32> } @__svml_idivrem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: [[REMAINDER:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 1
  // CHECK-AVX2: [[REMAINDER_CAST:%.*]] = bitcast <8 x i32> [[REMAINDER]] to <4 x i64>
  // CHECK-AVX2: store <4 x i64> [[REMAINDER_CAST]], ptr %{{.*}}
  // CHECK-AVX2: [[QUOTIENT:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 0
  // CHECK-AVX2: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
  // CHECK-AVX2: ret <4 x i64> [[QUOTIENT_CAST]]
  return _mm256_idivrem_epi32(A, B, C);
}

__m256i test_mm256_irem_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_irem_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_irem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_irem_epi32(A, B);
}

__m256i test_mm256_rem_epi8(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi8
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <32 x i8> @__svml_i8rem32(<32 x i8> [[DIVIDEND]], <32 x i8> [[DIVISOR]])
  // CHECK-AVX2: bitcast <32 x i8> [[RESULT]] to <4 x i64>
  return _mm256_rem_epi8(A, B);
}

__m256i test_mm256_rem_epu8(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu8
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <32 x i8>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <32 x i8> @__svml_u8rem32(<32 x i8> [[DIVIDEND]], <32 x i8> [[DIVISOR]])
  // CHECK-AVX2: bitcast <32 x i8> [[RESULT]] to <4 x i64>
  return _mm256_rem_epu8(A, B);
}

__m256i test_mm256_rem_epi16(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi16
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <16 x i16> @__svml_i16rem16(<16 x i16> [[DIVIDEND]], <16 x i16> [[DIVISOR]])
  // CHECK-AVX2: bitcast <16 x i16> [[RESULT]] to <4 x i64>
  return _mm256_rem_epi16(A, B);
}

__m256i test_mm256_rem_epu16(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu16
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <16 x i16> @__svml_u16rem16(<16 x i16> [[DIVIDEND]], <16 x i16> [[DIVISOR]])
  // CHECK-AVX2: bitcast <16 x i16> [[RESULT]] to <4 x i64>
  return _mm256_rem_epu16(A, B);
}

__m256i test_mm256_rem_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_irem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_rem_epi32(A, B);
}

__m256i test_mm256_rem_epu32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_urem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_rem_epu32(A, B);
}

__m256i test_mm256_rem_epi64(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epi64
  // CHECK-AVX2: call svml_cc <4 x i64> @__svml_i64rem4(<4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_rem_epi64(A, B);
}

__m256i test_mm256_rem_epu64(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_rem_epu64
  // CHECK-AVX2: call svml_cc <4 x i64> @__svml_u64rem4(<4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_rem_epu64(A, B);
}

__m256i test_mm256_udiv_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_udiv_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_udiv8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_udiv_epi32(A, B);
}

__m256i test_mm256_udivrem_epi32(__m256i *A, __m256i B, __m256i C) {
  // CHECK-AVX2-LABEL: test_mm256_udivrem_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc { <8 x i32>, <8 x i32> } @__svml_udivrem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: [[REMAINDER:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 1
  // CHECK-AVX2: [[REMAINDER_CAST:%.*]] = bitcast <8 x i32> [[REMAINDER]] to <4 x i64>
  // CHECK-AVX2: store <4 x i64> [[REMAINDER_CAST]], ptr %{{.*}}
  // CHECK-AVX2: [[QUOTIENT:%.*]] = extractvalue { <8 x i32>, <8 x i32> } [[RESULT]], 0
  // CHECK-AVX2: [[QUOTIENT_CAST:%.*]] = bitcast <8 x i32> [[QUOTIENT]] to <4 x i64>
  // CHECK-AVX2: ret <4 x i64> [[QUOTIENT_CAST]]
  return _mm256_udivrem_epi32(A, B, C);
}

__m256i test_mm256_urem_epi32(__m256i A, __m256i B) {
  // CHECK-AVX2-LABEL: test_mm256_urem_epi32
  // CHECK-AVX2: [[DIVIDEND:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[DIVISOR:%.*]] = bitcast <4 x i64> %{{.*}} to <8 x i32>
  // CHECK-AVX2: [[RESULT:%.*]] = call svml_cc <8 x i32> @__svml_urem8(<8 x i32> [[DIVIDEND]], <8 x i32> [[DIVISOR]])
  // CHECK-AVX2: bitcast <8 x i32> [[RESULT]] to <4 x i64>
  return _mm256_urem_epi32(A, B);
}
#endif // __AVX2__

// AVX512 single precision
#ifdef __AVX512F__
__m512 test_mm512_cbrt_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cbrt_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cbrtf16(<16 x float> %{{.*}})
  return _mm512_cbrt_ps(A);
}

__m512 test_mm512_mask_cbrt_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cbrt_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cbrtf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cbrt_ps(A, B, C);
}

__m512 test_mm512_exp_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_expf16(<16 x float> %{{.*}})
  return _mm512_exp_ps(A);
}

__m512 test_mm512_mask_exp_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_expf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_exp_ps(A, B, C);
}

__m512 test_mm512_exp10_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp10_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_exp10f16(<16 x float> %{{.*}})
  return _mm512_exp10_ps(A);
}

__m512 test_mm512_mask_exp10_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp10_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_exp10f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_exp10_ps(A, B, C);
}

__m512 test_mm512_exp2_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_exp2f16(<16 x float> %{{.*}})
  return _mm512_exp2_ps(A);
}

__m512 test_mm512_mask_exp2_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp2_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_exp2f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_exp2_ps(A, B, C);
}

__m512 test_mm512_expm1_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_expm1_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_expm1f16(<16 x float> %{{.*}})
  return _mm512_expm1_ps(A);
}

__m512 test_mm512_mask_expm1_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_expm1_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_expm1f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_expm1_ps(A, B, C);
}

__m512 test_mm512_hypot_ps(__m512 A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_hypot_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_hypotf16(<16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_hypot_ps(A, B);
}

__m512 test_mm512_mask_hypot_ps(__m512 A, __mmask16 B, __m512 C, __m512 D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_hypot_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_hypotf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_mask_hypot_ps(A, B, C, D);
}

__m512 test_mm512_invsqrt_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_invsqrt_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_invsqrtf16(<16 x float> %{{.*}})
  return _mm512_invsqrt_ps(A);
}

__m512 test_mm512_mask_invsqrt_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_invsqrt_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_invsqrtf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_invsqrt_ps(A, B, C);
}

__m512 test_mm512_log_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_logf16(<16 x float> %{{.*}})
  return _mm512_log_ps(A);
}

__m512 test_mm512_mask_log_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_logf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_log_ps(A, B, C);
}

__m512 test_mm512_log10_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log10_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log10f16(<16 x float> %{{.*}})
  return _mm512_log10_ps(A);
}

__m512 test_mm512_mask_log10_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log10_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log10f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_log10_ps(A, B, C);
}

__m512 test_mm512_log1p_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log1p_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log1pf16(<16 x float> %{{.*}})
  return _mm512_log1p_ps(A);
}

__m512 test_mm512_mask_log1p_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log1p_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log1pf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_log1p_ps(A, B, C);
}

__m512 test_mm512_log2_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_log2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log2f16(<16 x float> %{{.*}})
  return _mm512_log2_ps(A);
}

__m512 test_mm512_mask_log2_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log2_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_log2f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_log2_ps(A, B, C);
}

__m512 test_mm512_logb_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_logb_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_logbf16(<16 x float> %{{.*}})
  return _mm512_logb_ps(A);
}

__m512 test_mm512_mask_logb_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_logb_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_logbf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_logb_ps(A, B, C);
}

__m512 test_mm512_pow_ps(__m512 A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_pow_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_powf16(<16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_pow_ps(A, B);
}

__m512 test_mm512_mask_pow_ps(__m512 A, __mmask16 B, __m512 C, __m512 D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_pow_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_powf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_mask_pow_ps(A, B, C, D);
}

__m512 test_mm512_recip_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_recip_ps
  // CHECK-AVX512F: fdiv <16 x float> %{{.*}}, %{{.*}}
  return _mm512_recip_ps(A);
}

__m512 test_mm512_mask_recip_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_recip_ps
  // CHECK-AVX512F: fdiv <16 x float> %{{.*}}, %{{.*}}
  // CHECK-AVX512F: %{{.*}} = bitcast i16 %{{.*}} to <16 x i1>
  return _mm512_mask_recip_ps(A, B, C);
}

__m512 test_mm512_acos_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_acos_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acosf16(<16 x float> %{{.*}})
  return _mm512_acos_ps(A);
}

__m512 test_mm512_mask_acos_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_acos_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acosf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_acos_ps(A, B, C);
}

__m512 test_mm512_acosh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_acosh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acoshf16(<16 x float> %{{.*}})
  return _mm512_acosh_ps(A);
}

__m512 test_mm512_mask_acosh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_acosh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_acoshf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_acosh_ps(A, B, C);
}

__m512 test_mm512_asin_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_asin_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinf16(<16 x float> %{{.*}})
  return _mm512_asin_ps(A);
}

__m512 test_mm512_mask_asin_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_asin_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_asin_ps(A, B, C);
}

__m512 test_mm512_asinh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_asinh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinhf16(<16 x float> %{{.*}})
  return _mm512_asinh_ps(A);
}

__m512 test_mm512_mask_asinh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_asinh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_asinhf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_asinh_ps(A, B, C);
}

__m512 test_mm512_atan_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_atan_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanf16(<16 x float> %{{.*}})
  return _mm512_atan_ps(A);
}

__m512 test_mm512_mask_atan_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atan_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_atan_ps(A, B, C);
}

__m512 test_mm512_atan2_ps(__m512 A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_atan2_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atan2f16(<16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_atan2_ps(A, B);
}

__m512 test_mm512_mask_atan2_ps(__m512 A, __mmask16 B, __m512 C, __m512 D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atan2_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atan2f16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}}, <16 x float> %{{.*}})
  return _mm512_mask_atan2_ps(A, B, C, D);
}

__m512 test_mm512_atanh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_atanh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanhf16(<16 x float> %{{.*}})
  return _mm512_atanh_ps(A);
}

__m512 test_mm512_mask_atanh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atanh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_atanhf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_atanh_ps(A, B, C);
}

__m512 test_mm512_cos_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cos_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cosf16(<16 x float> %{{.*}})
  return _mm512_cos_ps(A);
}

__m512 test_mm512_mask_cos_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cos_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cosf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cos_ps(A, B, C);
}

__m512 test_mm512_cosd_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosd_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cosdf16(<16 x float> %{{.*}})
  return _mm512_cosd_ps(A);
}

__m512 test_mm512_mask_cosd_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cosd_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cosdf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cosd_ps(A, B, C);
}

__m512 test_mm512_cosh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_coshf16(<16 x float> %{{.*}})
  return _mm512_cosh_ps(A);
}

__m512 test_mm512_mask_cosh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cosh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_coshf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cosh_ps(A, B, C);
}

__m512 test_mm512_sin_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_sin_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinf16(<16 x float> %{{.*}})
  return _mm512_sin_ps(A);
}

__m512 test_mm512_mask_sin_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sin_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_sin_ps(A, B, C);
}

__m512 test_mm512_sincos_ps(__m512 *A, __m512 B) {
  // CHECK-AVX512F-LABEL: test_mm512_sincos_ps
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc { <16 x float>, <16 x float> } @__svml_sincosf16(<16 x float> %{{.*}})
  // CHECK-AVX512F: [[COS:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 1
  // CHECK-AVX512F: store <16 x float> [[COS]], ptr %{{.*}}
  // CHECK-AVX512F: [[SIN:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 0
  // CHECK-AVX512F: ret <16 x float> [[SIN]]
  return _mm512_sincos_ps(A, B);
}

__m512 test_mm512_mask_sincos_ps(__m512 *A, __m512 B, __m512 C, __mmask16 D, __m512 E) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sincos_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: [[SRC_TMP:%.*]] = insertvalue { <16 x float>, <16 x float> } undef, <16 x float> %{{.*}}, 0
  // CHECK-AVX512F: [[SRC:%.*]] = insertvalue { <16 x float>, <16 x float> } [[SRC_TMP]], <16 x float> %{{.*}}, 1
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc { <16 x float>, <16 x float> } @__svml_sincosf16_mask({ <16 x float>, <16 x float> } [[SRC]], <16 x i1> [[MASK]], <16 x float> %{{.*}})
  // CHECK-AVX512F: [[COS:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 1
  // CHECK-AVX512F: store <16 x float> [[COS]], ptr %{{.*}}
  // CHECK-AVX512F: [[SIN:%.*]] = extractvalue { <16 x float>, <16 x float> } [[RESULT]], 0
  // CHECK-AVX512F: ret <16 x float> [[SIN]]
  return _mm512_mask_sincos_ps(A, B, C, D, E);
}

__m512 test_mm512_sind_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_sind_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sindf16(<16 x float> %{{.*}})
  return _mm512_sind_ps(A);
}

__m512 test_mm512_mask_sind_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sind_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sindf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_sind_ps(A, B, C);
}

__m512 test_mm512_sinh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_sinh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinhf16(<16 x float> %{{.*}})
  return _mm512_sinh_ps(A);
}

__m512 test_mm512_mask_sinh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sinh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_sinhf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_sinh_ps(A, B, C);
}

__m512 test_mm512_tan_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_tan_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanf16(<16 x float> %{{.*}})
  return _mm512_tan_ps(A);
}

__m512 test_mm512_mask_tan_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tan_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_tan_ps(A, B, C);
}

__m512 test_mm512_tand_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_tand_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tandf16(<16 x float> %{{.*}})
  return _mm512_tand_ps(A);
}

__m512 test_mm512_mask_tand_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tand_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tandf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_tand_ps(A, B, C);
}

__m512 test_mm512_tanh_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_tanh_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanhf16(<16 x float> %{{.*}})
  return _mm512_tanh_ps(A);
}

__m512 test_mm512_mask_tanh_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tanh_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_tanhf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_tanh_ps(A, B, C);
}

__m512 test_mm512_cdfnorm_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cdfnorm_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cdfnormf16(<16 x float> %{{.*}})
  return _mm512_cdfnorm_ps(A);
}

__m512 test_mm512_mask_cdfnorm_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cdfnorm_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cdfnormf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cdfnorm_ps(A, B, C);
}

__m512 test_mm512_cdfnorminv_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_cdfnorminv_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cdfnorminvf16(<16 x float> %{{.*}})
  return _mm512_cdfnorminv_ps(A);
}

__m512 test_mm512_mask_cdfnorminv_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cdfnorminv_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_cdfnorminvf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_cdfnorminv_ps(A, B, C);
}

__m512 test_mm512_erf_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erf_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erff16(<16 x float> %{{.*}})
  return _mm512_erf_ps(A);
}

__m512 test_mm512_mask_erf_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erf_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erff16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_erf_ps(A, B, C);
}

__m512 test_mm512_erfc_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfc_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfcf16(<16 x float> %{{.*}})
  return _mm512_erfc_ps(A);
}

__m512 test_mm512_mask_erfc_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfc_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfcf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_erfc_ps(A, B, C);
}

__m512 test_mm512_erfcinv_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfcinv_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfcinvf16(<16 x float> %{{.*}})
  return _mm512_erfcinv_ps(A);
}

__m512 test_mm512_mask_erfcinv_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfcinv_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfcinvf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_erfcinv_ps(A, B, C);
}

__m512 test_mm512_erfinv_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfinv_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfinvf16(<16 x float> %{{.*}})
  return _mm512_erfinv_ps(A);
}

__m512 test_mm512_mask_erfinv_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfinv_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_erfinvf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_erfinv_ps(A, B, C);
}

__m512 test_mm512_nearbyint_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_nearbyint_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_nearbyintf16(<16 x float> %{{.*}})
  return _mm512_nearbyint_ps(A);
}

__m512 test_mm512_mask_nearbyint_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_nearbyint_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_nearbyintf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_nearbyint_ps(A, B, C);
}

__m512 test_mm512_rint_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_rint_ps
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_rintf16(<16 x float> %{{.*}})
  return _mm512_rint_ps(A);
}

__m512 test_mm512_mask_rint_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_rint_ps
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: call svml_cc <16 x float> @__svml_rintf16_mask(<16 x float> %{{.*}}, <16 x i1> [[MASK]], <16 x float> %{{.*}})
  return _mm512_mask_rint_ps(A, B, C);
}

__m512 test_mm512_ceil_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_ceil_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 2, <16 x float> {{.*}}, i16 -1, i32 4)
  return _mm512_ceil_ps(A);
}

__m512 test_mm512_mask_ceil_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_ceil_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 2, <16 x float> %{{.*}}, i16 %{{.*}}, i32 4)
  return _mm512_mask_ceil_ps(A, B, C);
}

__m512 test_mm512_floor_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_floor_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 1, <16 x float> {{.*}}, i16 -1, i32 4)
  return _mm512_floor_ps(A);
}

__m512 test_mm512_mask_floor_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_floor_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 1, <16 x float> %{{.*}}, i16 %{{.*}}, i32 4)
  return _mm512_mask_floor_ps(A, B, C);
}

__m512 test_mm512_trunc_ps(__m512 A) {
  // CHECK-AVX512F-LABEL: test_mm512_trunc_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 3, <16 x float> {{.*}}, i16 -1, i32 4)
  return _mm512_trunc_ps(A);
}

__m512 test_mm512_mask_trunc_ps(__m512 A, __mmask16 B, __m512 C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_trunc_ps
  // CHECK-AVX512F: call <16 x float> @llvm.x86.avx512.mask.rndscale.ps.512(<16 x float> %{{.*}}, i32 3, <16 x float> %{{.*}}, i16 %{{.*}}, i32 4)
  return _mm512_mask_trunc_ps(A, B, C);
}

// AVX512 double precision
__m512d test_mm512_cbrt_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cbrt_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cbrt8(<8 x double> %{{.*}})
  return _mm512_cbrt_pd(A);
}

__m512d test_mm512_mask_cbrt_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cbrt_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cbrt8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cbrt_pd(A, B, C);
}

__m512d test_mm512_exp_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp8(<8 x double> %{{.*}})
  return _mm512_exp_pd(A);
}

__m512d test_mm512_mask_exp_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_exp_pd(A, B, C);
}

__m512d test_mm512_exp10_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp10_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp108(<8 x double> %{{.*}})
  return _mm512_exp10_pd(A);
}

__m512d test_mm512_mask_exp10_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp10_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp108_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_exp10_pd(A, B, C);
}

__m512d test_mm512_exp2_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_exp2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp28(<8 x double> %{{.*}})
  return _mm512_exp2_pd(A);
}

__m512d test_mm512_mask_exp2_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_exp2_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_exp28_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_exp2_pd(A, B, C);
}

__m512d test_mm512_expm1_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_expm1_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_expm18(<8 x double> %{{.*}})
  return _mm512_expm1_pd(A);
}

__m512d test_mm512_mask_expm1_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_expm1_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_expm18_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_expm1_pd(A, B, C);
}

__m512d test_mm512_hypot_pd(__m512d A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_hypot_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_hypot8(<8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_hypot_pd(A, B);
}

__m512d test_mm512_mask_hypot_pd(__m512d A, __mmask8 B, __m512d C, __m512d D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_hypot_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_hypot8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_mask_hypot_pd(A, B, C, D);
}

__m512d test_mm512_invsqrt_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_invsqrt_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_invsqrt8(<8 x double> %{{.*}})
  return _mm512_invsqrt_pd(A);
}

__m512d test_mm512_mask_invsqrt_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_invsqrt_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_invsqrt8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_invsqrt_pd(A, B, C);
}

__m512d test_mm512_log_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log8(<8 x double> %{{.*}})
  return _mm512_log_pd(A);
}

__m512d test_mm512_mask_log_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_log_pd(A, B, C);
}

__m512d test_mm512_log10_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log10_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log108(<8 x double> %{{.*}})
  return _mm512_log10_pd(A);
}

__m512d test_mm512_mask_log10_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log10_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log108_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_log10_pd(A, B, C);
}

__m512d test_mm512_log1p_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log1p_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log1p8(<8 x double> %{{.*}})
  return _mm512_log1p_pd(A);
}

__m512d test_mm512_mask_log1p_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log1p_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log1p8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_log1p_pd(A, B, C);
}

__m512d test_mm512_log2_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_log2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log28(<8 x double> %{{.*}})
  return _mm512_log2_pd(A);
}

__m512d test_mm512_mask_log2_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_log2_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_log28_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_log2_pd(A, B, C);
}

__m512d test_mm512_logb_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_logb_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_logb8(<8 x double> %{{.*}})
  return _mm512_logb_pd(A);
}

__m512d test_mm512_mask_logb_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_logb_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_logb8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_logb_pd(A, B, C);
}

__m512d test_mm512_pow_pd(__m512d A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_pow_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_pow8(<8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_pow_pd(A, B);
}

__m512d test_mm512_mask_pow_pd(__m512d A, __mmask8 B, __m512d C, __m512d D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_pow_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_pow8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_mask_pow_pd(A, B, C, D);
}

__m512d test_mm512_recip_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_recip_pd
  // CHECK-AVX512F: fdiv <8 x double> %{{.*}}, %{{.*}}
  return _mm512_recip_pd(A);
}

__m512d test_mm512_mask_recip_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_recip_pd
  // CHECK-AVX512F: fdiv <8 x double> %{{.*}}, %{{.*}}
  // CHECK-AVX512F: %{{.*}} = bitcast i8 %{{.*}} to <8 x i1>
  return _mm512_mask_recip_pd(A, B, C);
}

__m512d test_mm512_acos_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_acos_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acos8(<8 x double> %{{.*}})
  return _mm512_acos_pd(A);
}

__m512d test_mm512_mask_acos_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_acos_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acos8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_acos_pd(A, B, C);
}

__m512d test_mm512_acosh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_acosh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acosh8(<8 x double> %{{.*}})
  return _mm512_acosh_pd(A);
}

__m512d test_mm512_mask_acosh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_acosh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_acosh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_acosh_pd(A, B, C);
}

__m512d test_mm512_asin_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_asin_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asin8(<8 x double> %{{.*}})
  return _mm512_asin_pd(A);
}

__m512d test_mm512_mask_asin_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_asin_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asin8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_asin_pd(A, B, C);
}

__m512d test_mm512_asinh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_asinh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asinh8(<8 x double> %{{.*}})
  return _mm512_asinh_pd(A);
}

__m512d test_mm512_mask_asinh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_asinh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_asinh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_asinh_pd(A, B, C);
}

__m512d test_mm512_atan_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_atan_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan8(<8 x double> %{{.*}})
  return _mm512_atan_pd(A);
}

__m512d test_mm512_mask_atan_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atan_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_atan_pd(A, B, C);
}

__m512d test_mm512_atan2_pd(__m512d A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_atan2_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan28(<8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_atan2_pd(A, B);
}

__m512d test_mm512_mask_atan2_pd(__m512d A, __mmask8 B, __m512d C, __m512d D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atan2_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atan28_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}}, <8 x double> %{{.*}})
  return _mm512_mask_atan2_pd(A, B, C, D);
}

__m512d test_mm512_atanh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_atanh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atanh8(<8 x double> %{{.*}})
  return _mm512_atanh_pd(A);
}

__m512d test_mm512_mask_atanh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_atanh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_atanh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_atanh_pd(A, B, C);
}

__m512d test_mm512_cos_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cos_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cos8(<8 x double> %{{.*}})
  return _mm512_cos_pd(A);
}

__m512d test_mm512_mask_cos_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cos_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cos8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cos_pd(A, B, C);
}

__m512d test_mm512_cosd_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosd_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cosd8(<8 x double> %{{.*}})
  return _mm512_cosd_pd(A);
}

__m512d test_mm512_mask_cosd_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cosd_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cosd8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cosd_pd(A, B, C);
}

__m512d test_mm512_cosh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cosh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cosh8(<8 x double> %{{.*}})
  return _mm512_cosh_pd(A);
}

__m512d test_mm512_mask_cosh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cosh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cosh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cosh_pd(A, B, C);
}

__m512d test_mm512_sin_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_sin_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sin8(<8 x double> %{{.*}})
  return _mm512_sin_pd(A);
}

__m512d test_mm512_mask_sin_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sin_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sin8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_sin_pd(A, B, C);
}

__m512d test_mm512_sincos_pd(__m512d *A, __m512d B) {
  // CHECK-AVX512F-LABEL: test_mm512_sincos_pd
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc { <8 x double>, <8 x double> } @__svml_sincos8(<8 x double> %{{.*}})
  // CHECK-AVX512F: [[COS:%.*]] = extractvalue { <8 x double>, <8 x double> } [[RESULT]], 1
  // CHECK-AVX512F: store <8 x double> [[COS]], ptr %{{.*}}
  // CHECK-AVX512F: [[SIN:%.*]] = extractvalue { <8 x double>, <8 x double> } [[RESULT]], 0
  // CHECK-AVX512F: ret <8 x double> [[SIN]]
  return _mm512_sincos_pd(A, B);
}

__m512d test_mm512_mask_sincos_pd(__m512d *A, __m512d B, __m512d C, __mmask8 D, __m512d E) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sincos_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: [[SRC_TMP:%.*]] = insertvalue { <8 x double>, <8 x double> } undef, <8 x double> %{{.*}}, 0
  // CHECK-AVX512F: [[SRC:%.*]] = insertvalue { <8 x double>, <8 x double> } [[SRC_TMP]], <8 x double> %{{.*}}, 1
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc { <8 x double>, <8 x double> } @__svml_sincos8_mask({ <8 x double>, <8 x double> } [[SRC]], <8 x i1> [[MASK]], <8 x double> %{{.*}})
  // CHECK-AVX512F: [[COS:%.*]] = extractvalue { <8 x double>, <8 x double> } [[RESULT]], 1
  // CHECK-AVX512F: store <8 x double> [[COS]], ptr %{{.*}}
  // CHECK-AVX512F: [[SIN:%.*]] = extractvalue { <8 x double>, <8 x double> } [[RESULT]], 0
  // CHECK-AVX512F: ret <8 x double> [[SIN]]
  return _mm512_mask_sincos_pd(A, B, C, D, E);
}

__m512d test_mm512_sind_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_sind_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sind8(<8 x double> %{{.*}})
  return _mm512_sind_pd(A);
}

__m512d test_mm512_mask_sind_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sind_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sind8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_sind_pd(A, B, C);
}

__m512d test_mm512_sinh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_sinh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sinh8(<8 x double> %{{.*}})
  return _mm512_sinh_pd(A);
}

__m512d test_mm512_mask_sinh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_sinh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_sinh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_sinh_pd(A, B, C);
}

__m512d test_mm512_tan_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_tan_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tan8(<8 x double> %{{.*}})
  return _mm512_tan_pd(A);
}

__m512d test_mm512_mask_tan_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tan_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tan8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_tan_pd(A, B, C);
}

__m512d test_mm512_tand_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_tand_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tand8(<8 x double> %{{.*}})
  return _mm512_tand_pd(A);
}

__m512d test_mm512_mask_tand_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tand_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tand8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_tand_pd(A, B, C);
}

__m512d test_mm512_tanh_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_tanh_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tanh8(<8 x double> %{{.*}})
  return _mm512_tanh_pd(A);
}

__m512d test_mm512_mask_tanh_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_tanh_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_tanh8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_tanh_pd(A, B, C);
}

__m512d test_mm512_cdfnorm_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cdfnorm_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cdfnorm8(<8 x double> %{{.*}})
  return _mm512_cdfnorm_pd(A);
}

__m512d test_mm512_mask_cdfnorm_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cdfnorm_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cdfnorm8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cdfnorm_pd(A, B, C);
}

__m512d test_mm512_cdfnorminv_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_cdfnorminv_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cdfnorminv8(<8 x double> %{{.*}})
  return _mm512_cdfnorminv_pd(A);
}

__m512d test_mm512_mask_cdfnorminv_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_cdfnorminv_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_cdfnorminv8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_cdfnorminv_pd(A, B, C);
}

__m512d test_mm512_erf_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erf_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erf8(<8 x double> %{{.*}})
  return _mm512_erf_pd(A);
}

__m512d test_mm512_mask_erf_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erf_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erf8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_erf_pd(A, B, C);
}

__m512d test_mm512_erfc_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfc_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfc8(<8 x double> %{{.*}})
  return _mm512_erfc_pd(A);
}

__m512d test_mm512_mask_erfc_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfc_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfc8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_erfc_pd(A, B, C);
}

__m512d test_mm512_erfcinv_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfcinv_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfcinv8(<8 x double> %{{.*}})
  return _mm512_erfcinv_pd(A);
}

__m512d test_mm512_mask_erfcinv_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfcinv_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfcinv8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_erfcinv_pd(A, B, C);
}

__m512d test_mm512_erfinv_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_erfinv_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfinv8(<8 x double> %{{.*}})
  return _mm512_erfinv_pd(A);
}

__m512d test_mm512_mask_erfinv_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_erfinv_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_erfinv8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_erfinv_pd(A, B, C);
}

__m512d test_mm512_nearbyint_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_nearbyint_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_nearbyint8(<8 x double> %{{.*}})
  return _mm512_nearbyint_pd(A);
}

__m512d test_mm512_mask_nearbyint_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_nearbyint_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_nearbyint8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_nearbyint_pd(A, B, C);
}

__m512d test_mm512_rint_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_rint_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_rint8(<8 x double> %{{.*}})
  return _mm512_rint_pd(A);
}

__m512d test_mm512_mask_rint_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_rint_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_rint8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_rint_pd(A, B, C);
}

__m512d test_mm512_svml_round_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_svml_round_pd
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_round8(<8 x double> %{{.*}})
  return _mm512_svml_round_pd(A);
}

__m512d test_mm512_mask_svml_round_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_svml_round_pd
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-AVX512F: call svml_cc <8 x double> @__svml_round8_mask(<8 x double> %{{.*}}, <8 x i1> [[MASK]], <8 x double> %{{.*}})
  return _mm512_mask_svml_round_pd(A, B, C);
}

__m512d test_mm512_ceil_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_ceil_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 2, <8 x double> {{.*}}, i8 -1, i32 4)
  return _mm512_ceil_pd(A);
}

__m512d test_mm512_mask_ceil_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_ceil_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 2, <8 x double> %{{.*}}, i8 %{{.*}}, i32 4)
  return _mm512_mask_ceil_pd(A, B, C);
}

__m512d test_mm512_floor_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_floor_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 1, <8 x double> {{.*}}, i8 -1, i32 4)
  return _mm512_floor_pd(A);
}

__m512d test_mm512_mask_floor_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_floor_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 1, <8 x double> %{{.*}}, i8 %{{.*}}, i32 4)
  return _mm512_mask_floor_pd(A, B, C);
}

__m512d test_mm512_trunc_pd(__m512d A) {
  // CHECK-AVX512F-LABEL: test_mm512_trunc_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 3, <8 x double> {{.*}}, i8 -1, i32 4)
  return _mm512_trunc_pd(A);
}

__m512d test_mm512_mask_trunc_pd(__m512d A, __mmask8 B, __m512d C) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_trunc_pd
  // CHECK-AVX512F: call <8 x double> @llvm.x86.avx512.mask.rndscale.pd.512(<8 x double> %{{.*}}, i32 3, <8 x double> %{{.*}}, i8 %{{.*}}, i32 4)
  return _mm512_mask_trunc_pd(A, B, C);
}

// AVX512 Int
__m512i test_mm512_div_epi8(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi8
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <64 x i8> @__svml_i8div64(<64 x i8> [[DIVIDEND]], <64 x i8> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <64 x i8> [[RESULT]] to <8 x i64>
  return _mm512_div_epi8(A, B);
}

__m512i test_mm512_div_epu8(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu8
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <64 x i8> @__svml_u8div64(<64 x i8> [[DIVIDEND]], <64 x i8> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <64 x i8> [[RESULT]] to <8 x i64>
  return _mm512_div_epu8(A, B);
}

__m512i test_mm512_div_epi16(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi16
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <32 x i16> @__svml_i16div32(<32 x i16> [[DIVIDEND]], <32 x i16> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <32 x i16> [[RESULT]] to <8 x i64>
  return _mm512_div_epi16(A, B);
}

__m512i test_mm512_div_epu16(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu16
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <32 x i16> @__svml_u16div32(<32 x i16> [[DIVIDEND]], <32 x i16> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <32 x i16> [[RESULT]] to <8 x i64>
  return _mm512_div_epu16(A, B);
}

__m512i test_mm512_div_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi32
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_idiv16(<16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_div_epi32(A, B);
}

__m512i test_mm512_div_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu32
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_udiv16(<16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_div_epu32(A, B);
}

__m512i test_mm512_mask_div_epi32(__m512i A, __mmask16 B, __m512i C, __m512i D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_div_epi32
  // CHECK-AVX512F: [[SRC:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_idiv16_mask(<16 x i32> [[SRC]], <16 x i1> [[MASK]], <16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_mask_div_epi32(A, B, C, D);
}

__m512i test_mm512_mask_div_epu32(__m512i A, __mmask16 B, __m512i C, __m512i D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_div_epu32
  // CHECK-AVX512F: [[SRC:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_udiv16_mask(<16 x i32> [[SRC]], <16 x i1> [[MASK]], <16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_mask_div_epu32(A, B, C, D);
}

__m512i test_mm512_div_epi64(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epi64
  // CHECK-AVX512F: call svml_cc <8 x i64> @__svml_i64div8(<8 x i64> %{{.*}}, <8 x i64> %{{.*}})
  return _mm512_div_epi64(A, B);
}

__m512i test_mm512_div_epu64(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_div_epu64
  // CHECK-AVX512F: call svml_cc <8 x i64> @__svml_u64div8(<8 x i64> %{{.*}}, <8 x i64> %{{.*}})
  return _mm512_div_epu64(A, B);
}

__m512i test_mm512_rem_epi8(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi8
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <64 x i8> @__svml_i8rem64(<64 x i8> [[DIVIDEND]], <64 x i8> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <64 x i8> [[RESULT]] to <8 x i64>
  return _mm512_rem_epi8(A, B);
}

__m512i test_mm512_rem_epu8(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu8
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <64 x i8>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <64 x i8> @__svml_u8rem64(<64 x i8> [[DIVIDEND]], <64 x i8> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <64 x i8> [[RESULT]] to <8 x i64>
  return _mm512_rem_epu8(A, B);
}

__m512i test_mm512_rem_epi16(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi16
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <32 x i16> @__svml_i16rem32(<32 x i16> [[DIVIDEND]], <32 x i16> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <32 x i16> [[RESULT]] to <8 x i64>
  return _mm512_rem_epi16(A, B);
}

__m512i test_mm512_rem_epu16(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu16
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <32 x i16> @__svml_u16rem32(<32 x i16> [[DIVIDEND]], <32 x i16> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <32 x i16> [[RESULT]] to <8 x i64>
  return _mm512_rem_epu16(A, B);
}

__m512i test_mm512_rem_epi32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi32
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_irem16(<16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_rem_epi32(A, B);
}

__m512i test_mm512_rem_epu32(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu32
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_urem16(<16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_rem_epu32(A, B);
}

__m512i test_mm512_mask_rem_epi32(__m512i A, __mmask16 B, __m512i C, __m512i D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_rem_epi32
  // CHECK-AVX512F: [[SRC:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_irem16_mask(<16 x i32> [[SRC]], <16 x i1> [[MASK]], <16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_mask_rem_epi32(A, B, C, D);
}

__m512i test_mm512_mask_rem_epu32(__m512i A, __mmask16 B, __m512i C, __m512i D) {
  // CHECK-AVX512F-LABEL: test_mm512_mask_rem_epu32
  // CHECK-AVX512F: [[SRC:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[MASK:%.*]] = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK-AVX512F: [[DIVIDEND:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[DIVISOR:%.*]] = bitcast <8 x i64> %{{.*}} to <16 x i32>
  // CHECK-AVX512F: [[RESULT:%.*]] = call svml_cc <16 x i32> @__svml_urem16_mask(<16 x i32> [[SRC]], <16 x i1> [[MASK]], <16 x i32> [[DIVIDEND]], <16 x i32> [[DIVISOR]])
  // CHECK-AVX512F: bitcast <16 x i32> [[RESULT]] to <8 x i64>
  return _mm512_mask_rem_epu32(A, B, C, D);
}

__m512i test_mm512_rem_epi64(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epi64
  // CHECK-AVX512F: call svml_cc <8 x i64> @__svml_i64rem8(<8 x i64> %{{.*}}, <8 x i64> %{{.*}})
  return _mm512_rem_epi64(A, B);
}

__m512i test_mm512_rem_epu64(__m512i A, __m512i B) {
  // CHECK-AVX512F-LABEL: test_mm512_rem_epu64
  // CHECK-AVX512F: call svml_cc <8 x i64> @__svml_u64rem8(<8 x i64> %{{.*}}, <8 x i64> %{{.*}})
  return _mm512_rem_epu64(A, B);
}

#endif // __AVX512F__
