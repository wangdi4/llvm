// REQUIRES: intel_feature_isa_avx512_dotprod
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512convert -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_m128_mask_vcvt2ps2ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.128
__m128h test_m128_mask_vcvt2ps2ph_ph( __m128h __W, __mmask8 __U, __m128 __A, __m128 __B) {
  return _m128_mask_vcvt2ps2ph_ph(__W, __U, __A, __B);
}

// CHECK-LABEL: @test_m128_mask_vcvtbf162ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx512.mask.vcvtbf162ph.128
__m128h test_m128_mask_vcvtbf162ph_ph(__m128h __W, __mmask8 __U, __m128i __A) {
  return _m128_mask_vcvtbf162ph_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_m128_mask_vcvtneph2bf16_ph(
// CHECK:    call <8 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.128
__m128i test_m128_mask_vcvtneph2bf16_ph(__m128i __W, __mmask8 __U, __m128h __A) {
  return _m128_mask_vcvtneph2bf16_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_m256_mask_vcvt2ps2ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.256
__m256h test_m256_mask_vcvt2ps2ph_ph( __m256h __W, __mmask16 __U, __m256 __A, __m256 __B) {
  return _m256_mask_vcvt2ps2ph_ph(__W, __U, __A, __B);
}

// CHECK-LABEL: @test_m256_mask_vcvtbf162ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx512.mask.vcvtbf162ph.256
__m256h test_m256_mask_vcvtbf162ph_ph(__m256h __W, __mmask16 __U, __m256i __A) {
  return _m256_mask_vcvtbf162ph_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_m256_mask_vcvtneph2bf16_ph(
// CHECK:    call <16 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.256
__m256i test_m256_mask_vcvtneph2bf16_ph(__m256i __W, __mmask16 __U, __m256h __A) {
  return _m256_mask_vcvtneph2bf16_ph(__W, __U, __A);
}

// CHECK-LABEL: @test_m128_maskz_vcvt2ps2ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.128
__m128h test_m128_maskz_vcvt2ps2ph_ph(__mmask8 __U, __m128 __A, __m128 __B) {
  return _m128_maskz_vcvt2ps2ph_ph(__U, __A, __B);
}

// CHECK-LABEL: @test_m128_maskz_vcvtbf162ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx512.mask.vcvtbf162ph.128
__m128h test_m128_maskz_vcvtbf162ph_ph(__mmask8 __U, __m128i __A) {
  return _m128_maskz_vcvtbf162ph_ph(__U, __A);
}

// CHECK-LABEL: @test_m128_maskz_vcvtneph2bf16_ph(
// CHECK:    call <8 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.128
__m128i test_m128_maskz_vcvtneph2bf16_ph(__mmask8 __U, __m128h __A) {
  return _m128_maskz_vcvtneph2bf16_ph(__U, __A);
}

// CHECK-LABEL: @test_m256_maskz_vcvt2ps2ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx512.mask.vcvt2ps2ph.256
__m256h test_m256_maskz_vcvt2ps2ph_ph(__mmask16 __U, __m256 __A, __m256 __B) {
  return _m256_maskz_vcvt2ps2ph_ph(__U, __A, __B);
}

// CHECK-LABEL: @test_m256_maskz_vcvtbf162ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx512.mask.vcvtbf162ph.256
__m256h test_m256_maskz_vcvtbf162ph_ph(__mmask16 __U, __m256i __A) {
  return _m256_maskz_vcvtbf162ph_ph(__U, __A);
}

// CHECK-LABEL: @test_m256_maskz_vcvtneph2bf16_ph(
// CHECK:    call <16 x i16> @llvm.x86.avx512.mask.vcvtneph2bf16.256
__m256i test_m256_maskz_vcvtneph2bf16_ph(__mmask16 __U, __m256h __A) {
  return _m256_maskz_vcvtneph2bf16_ph(__U, __A);
}
