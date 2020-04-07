// REQUIRES: intel_feature_isa_avx_dotprod
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avxconvert -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_m128_vcvt2ps2ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx2.vcvt2ps2ph.128
__m128h test_m128_vcvt2ps2ph_ph(__m128 __A, __m128 __B) {
  return _m128_vcvt2ps2ph_ph(__A, __B);
}

// CHECK-LABEL: @test_m128_vcvtbf162ph_ph(
// CHECK:    call <8 x half> @llvm.x86.avx2.vcvtbf162ph.128
__m128h test_m128_vcvtbf162ph_ph(__m128i __A) {
  return _m128_vcvtbf162ph_ph(__A);
}

// CHECK-LABEL: @test_m128_vcvtneph2bf16_ph(
// CHECK:    call <8 x i16> @llvm.x86.avx2.vcvtneph2bf16.128
__m128i test_m128_vcvtneph2bf16_ph(__m128h __A) {
  return _m128_vcvtneph2bf16_ph(__A);
}

// CHECK-LABEL: @test_m256_vcvt2ps2ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx2.vcvt2ps2ph.256
__m256h test_m256_vcvt2ps2ph_ph(__m256 __A, __m256 __B) {
  return _m256_vcvt2ps2ph_ph(__A, __B);
}

// CHECK-LABEL: @test_m256_vcvtbf162ph_ph(
// CHECK:    call <16 x half> @llvm.x86.avx2.vcvtbf162ph.256
__m256h test_m256_vcvtbf162ph_ph(__m256i __A) {
  return _m256_vcvtbf162ph_ph(__A);
}

// CHECK-LABEL: @test_m256_vcvtneph2bf16_ph(
// CHECK:    call <16 x i16> @llvm.x86.avx2.vcvtneph2bf16.256
__m256i test_m256_vcvtneph2bf16_ph(__m256h __A) {
  return _m256_vcvtneph2bf16_ph(__A);
}
