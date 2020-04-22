// REQUIRES: intel_feature_isa_avx512_dotprod
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512convert -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_m512_mask_vcvt2ps2ph_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.512
__m512h test_m512_mask_vcvt2ps2ph_ph( __m512h __W, __mmask32 __U, __m512 __A, __m512 __B) {
  return _m512_mask_vcvt2ps2ph_ph(__W, __U, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_vcvt2ps2ph_round_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.512
__m512h test_mm512_mask_vcvt2ps2ph_round_ph(__m512h __W, __mmask32 __U, __m512 __A, __m512 __B) {
  return _mm512_mask_vcvt2ps2ph_round_ph(__W, __U, __A, __B, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_m512_mask_vcvtbf162ph_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvtbf162ph.512
__m512h test_m512_mask_vcvtbf162ph_ph(__m512h __W, __mmask32 __U, __m512i __A) {
  return _m512_mask_vcvtbf162ph_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_mm512_mask_vcvtbf162ph_round_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvtbf162ph.512
__m512h test_mm512_mask_vcvtbf162ph_round_ph(__m512h __W, __mmask32 __U, __m512i __A) {
  return _mm512_mask_vcvtbf162ph_round_ph(__W, __U, __A, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_m512_mask_vcvtneph2bf16_ph(
// CHECK:    call <32 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.512
__m512i test_m512_mask_vcvtneph2bf16_ph(__m512i __W, __mmask32 __U, __m512h __A) {
  return _m512_mask_vcvtneph2bf16_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_m512_maskz_vcvt2ps2ph_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.512
__m512h test_m512_maskz_vcvt2ps2ph_ph(__mmask32 __U, __m512 __A, __m512 __B) {
  return _m512_maskz_vcvt2ps2ph_ph(__U, __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_vcvt2ps2ph_round_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.512
__m512h test_mm512_maskz_vcvt2ps2ph_round_ph(__mmask32 __U, __m512 __A, __m512 __B) {
  return _mm512_maskz_vcvt2ps2ph_round_ph(__U, __A, __B, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_m512_maskz_vcvtbf162ph_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvtbf162ph.512
__m512h test_m512_maskz_vcvtbf162ph_ph(__mmask32 __U, __m512i __A) {
  return _m512_maskz_vcvtbf162ph_ph(__U, __A);
}

// CHECK-LABEL: @test_mm512_maskz_vcvtbf162ph_round_ph(
// CHECK:    call <32 x half> @llvm.x86.avx512.mask.vcvtbf162ph.512
__m512h test_mm512_maskz_vcvtbf162ph_round_ph(__mmask32 __U, __m512i __A) {
  return _mm512_maskz_vcvtbf162ph_round_ph(__U, __A, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}

// CHECK-LABEL: @test_m512_maskz_vcvtneph2bf16_ph(
// CHECK:    call <32 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.512
__m512i test_m512_maskz_vcvtneph2bf16_ph(__mmask32 __U, __m512h __A) {
  return _m512_maskz_vcvtneph2bf16_ph(__U, __A);
}
