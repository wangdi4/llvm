// REQUIRES: intel_feature_isa_avx512_vnni_fp16
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vnnifp16 -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m512 test_mm512_dpph_ps(__m512 __W, __m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_dpph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vdpphps.512
  return _mm512_dpph_ps(__W, __A, __B);
}

__m512 test_mm512_mask_dpph_ps(__m512 __W, __mmask16 __U, __m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_mask_dpph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vdpphps.512
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_mask_dpph_ps(__W, __U, __A, __B);
}

__m512 test_mm512_maskz_dpph_ps(__mmask16 __U, __m512 __W, __m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_maskz_dpph_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512.vdpphps.512
  // CHECK: zeroinitializer
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_maskz_dpph_ps(__U, __W, __A, __B);
}
