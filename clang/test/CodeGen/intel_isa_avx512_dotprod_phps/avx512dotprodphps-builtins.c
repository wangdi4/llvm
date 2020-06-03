// REQUIRES: intel_feature_isa_avx512_dotprod_phps
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512dotprodphps -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_mm512_vdpphps_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.mask.vdpphps.512
__m512 test_mm512_vdpphps_ps(__m512 __W, __m512h __A, __m512h __B) {
  return _mm512_vdpphps_ps(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_vdpphps_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.mask.vdpphps.512
__m512 test_mm512_mask_vdpphps_ps(__m512 __W, __mmask16 __U, __m512h __A, __m512h __B) {
  return _mm512_mask_vdpphps_ps(__W, __U, __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_vdpphps_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.maskz.vdpphps.512
__m512 test_mm512_maskz_vdpphps_ps(__mmask16 __U, __m512 __W, __m512h __A, __m512h __B) {
  return _mm512_maskz_vdpphps_ps(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_vdpphps_round_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.mask.vdpphps.512
__m512 test_mm512_vdpphps_round_ps(__m512 __W, __m512h __A, __m512h __B) {
  return _mm512_vdpphps_round_ps(__W, __A, __B, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_mm512_mask_vdpphps_round_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.mask.vdpphps.512
__m512 test_mm512_mask_vdpphps_round_ps(__m512 __W, __mmask16 __U, __m512h __A, __m512h __B) {
  return _mm512_mask_vdpphps_round_ps(__W, __U, __A, __B, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_mm512_maskz_vdpphps_round_ps(
// CHECK:     call <16 x float> @llvm.x86.avx512.maskz.vdpphps.512
__m512 test_mm512_maskz_vdpphps_round_ps(__mmask16 __U, __m512 __W, __m512h __A, __m512h __B) {
  return _mm512_maskz_vdpphps_round_ps(__U, __W, __A, __B, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
