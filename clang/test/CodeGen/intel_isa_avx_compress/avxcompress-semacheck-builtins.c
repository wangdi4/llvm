// REQUIRES: intel_feature_isa_avx_compress
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +avxcompress -emit-llvm -fsyntax-only -verify

#include <immintrin.h>

__m256i test_mm256_avxcompress_shldi_epi64(__m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshldq256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shldi_epi64( __m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshldq128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_shldi_epi32(__m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshldd256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shldi_epi32(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshldd128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_shldi_epi16( __m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshldw256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shldi_epi16(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshldw128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_shrdi_epi64(__m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshrdq256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shrdi_epi64(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshrdq128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_shrdi_epi32(__m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshrdd256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shrdi_epi32(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshrdd128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_shrdi_epi16(__m256i __A, __m256i __B) {
  return __builtin_ia32_avxcompress_vpshrdw256(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_shrdi_epi16(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshrdw128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm128_avxcompress_vprold128(__m128i __A, __m128i __B) {
  return __builtin_ia32_avxcompress_vpshrdw128(__A, __B, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm_avxcompress_rol_epi32(__m128i __A) {
  return __builtin_ia32_avxcompress_vprold128(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_rol_epi32(__m256i __A) {
  return __builtin_ia32_avxcompress_vprold256(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm_avxcompress_rol_epi64(__m128i __A) {
  return __builtin_ia32_avxcompress_vprolq128(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_rol_epi64(__m256i __A) {
  return __builtin_ia32_avxcompress_vprolq256(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm_avxcompress_ror_epi32(__m128i __A) {
  return __builtin_ia32_avxcompress_vprord128(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_ror_epi32(__m256i __A) {
  return __builtin_ia32_avxcompress_vprord256(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m128i test_mm_avxcompress_ror_epi64(__m128i __A) {
  return __builtin_ia32_avxcompress_vprorq128(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}

__m256i test_mm256_avxcompress_ror_epi64(__m256i __A) {
  return __builtin_ia32_avxcompress_vprorq256(__A, 1024); // expected-error {{argument value 1024 is outside the valid range [0, 255]}}
}
