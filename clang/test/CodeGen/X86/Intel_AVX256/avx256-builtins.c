// REQUIRES: intel_feature_isa_avx256
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-cpu common-avx256 -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %s

#include <immintrin.h>

__m128d test_mm_sqrt_pd(__m128d __A) {
  // CHECK-LABEL: @test_mm_sqrt_pd
  // CHECK: @llvm.sqrt.v2f64
  return _mm_sqrt_pd(__A);
}
__m256d test_mm256_sqrt_pd(__m256d __A) {
  // CHECK-LABEL: @test_mm256_sqrt_pd
  // CHECK: @llvm.sqrt.v4f64
  return _mm256_sqrt_pd(__A);
}
__m128d test_mm_mask_sqrt_pd(__m128d __W, __mmask8 __U, __m128d __A) {
  // CHECK-LABEL: @test_mm_mask_sqrt_pd
  // CHECK: @llvm.sqrt.v2f64
  // CHECK: select <2 x i1> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}
  return _mm_mask_sqrt_pd(__W,__U,__A);
}
__m128d test_mm_maskz_sqrt_pd(__mmask8 __U, __m128d __A) {
  // CHECK-LABEL: @test_mm_maskz_sqrt_pd
  // CHECK: @llvm.sqrt.v2f64
  // CHECK: select <2 x i1> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}
  return _mm_maskz_sqrt_pd(__U,__A);
}
__m256d test_mm256_mask_sqrt_pd(__m256d __W, __mmask8 __U, __m256d __A) {
  // CHECK-LABEL: @test_mm256_mask_sqrt_pd
  // CHECK: @llvm.sqrt.v4f64
  // CHECK: select <4 x i1> %{{.*}}, <4 x double> %{{.*}}, <4 x double> %{{.*}}
  return _mm256_mask_sqrt_pd(__W,__U,__A);
}
__m256d test_mm256_maskz_sqrt_pd(__mmask8 __U, __m256d __A) {
  // CHECK-LABEL: @test_mm256_maskz_sqrt_pd
  // CHECK: @llvm.sqrt.v4f64
  // CHECK: select <4 x i1> %{{.*}}, <4 x double> %{{.*}}, <4 x double> %{{.*}}
  return _mm256_maskz_sqrt_pd(__U,__A);
}

__m128 test_mm_cvtpd_ps(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtpd_ps
  // CHECK: @llvm.x86.sse2.cvtpd2ps
  return _mm_cvtpd_ps(__A);
}
__m128 test_mm_mask_cvtpd_ps(__m128 __W, __mmask8 __U, __m128d __A) {
  // CHECK-LABEL: @test_mm_mask_cvtpd_ps
  // CHECK: @llvm.x86.avx512.mask.cvtpd2ps
  return _mm_mask_cvtpd_ps(__W,__U,__A);
}
