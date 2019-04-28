// REQUIRES: intel_feature_isa_fp16
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m128h test_mm_setzero_ph()
{
  // CHECK-LABEL: @test_mm_setzero_ph
  // CHECK: zeroinitializer
  return _mm_setzero_ph();
}

__m256h test_mm256_setzero_ph()
{
  // CHECK-LABEL: @test_mm256_setzero_ph
  // CHECK: zeroinitializer
  return _mm256_setzero_ph();
}

__m256h test_mm256_undefined_ph() {
  // CHECK-LABEL: @test_mm256_undefined_ph
  // CHECK: ret <16 x half> zeroinitializer
  return _mm256_undefined_ph();
}

__m512h test_mm512_setzero_ph()
{
  // CHECK-LABEL: @test_mm512_setzero_ph
  // CHECK: zeroinitializer
  return _mm512_setzero_ph();
}

__m128h test_mm_undefined_ph() {
  // CHECK-LABEL: @test_mm_undefined_ph
  // CHECK: ret <8 x half> zeroinitializer
  return _mm_undefined_ph();
}

__m512h test_mm512_undefined_ph() {
  // CHECK-LABEL: @test_mm512_undefined_ph
  // CHECK: ret <32 x half> zeroinitializer
  return _mm512_undefined_ph();
}