// REQUIRES: intel_feature_isa_avx512_mediax
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx512mediax -target-feature +avx512vl -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// vmpsadbw128
__m128i test_mm_mpsadbw_epu8(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mpsadbw_epu8
  // CHECK: @llvm.x86.sse41.mpsadbw
  return _mm_mpsadbw_epu8(__A, __B, 170);
}

__m128i test_mm_mask_mpsadbw_epu8(__m128i __W, __mmask8 __U, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_mask_mpsadbw_epu8
  // CHECK: @llvm.x86.sse41.mpsadbw
  // CHECK: select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
  return _mm_mask_mpsadbw_epu8(__W, __U, __A, __B, 170);
}

__m128i test_mm_maskz_mpsadbw_epu8(__mmask8 __U, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_maskz_mpsadbw_epu8
  // CHECK: @llvm.x86.sse41.mpsadbw
  // CHECK: select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
  return _mm_maskz_mpsadbw_epu8(__U, __A, __B, 170);
}

// vmpsadbw256
__m256i test_mm256_mpsadbw_epu8(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mpsadbw_epu8
  // CHECK: @llvm.x86.avx2.mpsadbw
  return _mm256_mpsadbw_epu8(__A, __B, 170);
}

__m256i test_mm256_mask_mpsadbw_epu8(__m256i __W, __mmask16 __U, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_mask_mpsadbw_epu8
  // CHECK: @llvm.x86.avx2.mpsadbw
  // CHECK: select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
  return _mm256_mask_mpsadbw_epu8(__W, __U, __A, __B, 170);
}

__m256i test_mm256_maskz_mpsadbw_epu8(__mmask16 __U, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_maskz_mpsadbw_epu8
  // CHECK: @llvm.x86.avx2.mpsadbw
  // CHECK: select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
  return _mm256_maskz_mpsadbw_epu8(__U, __A, __B, 170);
}

// vmpsadbw512
__m512i test_mm512_mpsadbw_epu8(__m512i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mpsadbw_epu8
  // CHECK: @llvm.x86.avx512.mpsadbw.512
  return _mm512_mpsadbw_epu8(__A, __B, 170);
}

__m512i test_mm512_mask_mpsadbw_epu8(__m512i __W, __mmask32 __U, __m512i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_mask_mpsadbw_epu8
  // CHECK: @llvm.x86.avx512.mpsadbw.512
  //CHECK: select <32 x i1> %{{.*}}, <32 x i16> %{{.*}}, <32 x i16> %{{.*}}
  return _mm512_mask_mpsadbw_epu8(__W, __U, __A, __B, 170);
}

__m512i test_mm512_maskz_mpsadbw_epu8(__mmask32 __U, __m512i __A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_maskz_mpsadbw_epu8
  // CHECK: @llvm.x86.avx512.mpsadbw.512
  //CHECK: select <32 x i1> %{{.*}}, <32 x i16> %{{.*}}, <32 x i16> %{{.*}}
  return _mm512_maskz_mpsadbw_epu8(__U, __A, __B, 170);
}
