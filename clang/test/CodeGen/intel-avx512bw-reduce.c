// RUN: %clang_cc1 -ffreestanding %s -O0 -triple=x86_64-apple-darwin -target-cpu skylake-avx512 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

short test_mm_reduce_add_epi16(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_add_epi16(
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_add_epi16(__W);
}

short test_mm_reduce_mul_epi16(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_mul_epi16(
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_mul_epi16(__W);
}

short test_mm_reduce_or_epi16(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_or_epi16(
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_or_epi16(__W);
}

short test_mm_reduce_and_epi16(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_and_epi16(
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_reduce_and_epi16(__W);
}

short test_mm_mask_reduce_add_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_add_epi16(
// CHECK:    bitcast i8 %{{.*}} to <8 x i1>
// CHECK:    select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_add_epi16(__M, __W);
}

short test_mm_mask_reduce_mul_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_mul_epi16(
// CHECK:    bitcast i8 %{{.*}} to <8 x i1>
// CHECK:    select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_mul_epi16(__M, __W);
}

short test_mm_mask_reduce_and_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_and_epi16(
// CHECK:    bitcast i8 %{{.*}} to <8 x i1>
// CHECK:    select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_and_epi16(__M, __W);
}

short test_mm_mask_reduce_or_epi16(__mmask8 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_or_epi16(
// CHECK:    bitcast i8 %{{.*}} to <8 x i1>
// CHECK:    select <8 x i1> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm_mask_reduce_or_epi16(__M, __W);
}

short test_mm256_reduce_add_epi16(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_add_epi16(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_add_epi16(__W);
}

short test_mm256_reduce_mul_epi16(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_mul_epi16(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_mul_epi16(__W);
}

short test_mm256_reduce_or_epi16(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_or_epi16(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_or_epi16(__W);
}

short test_mm256_reduce_and_epi16(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_and_epi16(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_reduce_and_epi16(__W);
}

short test_mm256_mask_reduce_add_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_add_epi16(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    add <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_add_epi16(__M, __W);
}

short test_mm256_mask_reduce_mul_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_mul_epi16(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    mul <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_mul_epi16(__M, __W);
}

short test_mm256_mask_reduce_and_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_and_epi16(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    and <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_and_epi16(__M, __W);
}

short test_mm256_mask_reduce_or_epi16(__mmask16 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_or_epi16(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
// CHECK:    or <8 x i16> %{{.*}}, %{{.*}}
// CHECK:    extractelement <8 x i16> %{{.*}}, i32 0
  return _mm256_mask_reduce_or_epi16(__M, __W);
}

char test_mm_reduce_add_epi8(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_add_epi8(
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_add_epi8(__W);
}

char test_mm_reduce_mul_epi8(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_mul_epi8(
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_mul_epi8(__W);
}

char test_mm_reduce_and_epi8(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_and_epi8(
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_and_epi8(__W);
}

char test_mm_reduce_or_epi8(__m128i __W){
// CHECK-LABEL: @test_mm_reduce_or_epi8(
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_reduce_or_epi8(__W);
}

char test_mm_mask_reduce_add_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_add_epi8(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_add_epi8(__M, __W);
}

char test_mm_mask_reduce_mul_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_mul_epi8(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_mul_epi8(__M, __W);
}

char test_mm_mask_reduce_and_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_and_epi8(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_and_epi8(__M, __W);
}

char test_mm_mask_reduce_or_epi8(__mmask16 __M, __m128i __W){
// CHECK-LABEL: @test_mm_mask_reduce_or_epi8(
// CHECK:    bitcast i16 %{{.*}} to <16 x i1>
// CHECK:    select <16 x i1> %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm_mask_reduce_or_epi8(__M, __W);
}

char test_mm256_reduce_add_epi8(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_add_epi8(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_add_epi8(__W);
}

char test_mm256_reduce_mul_epi8(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_mul_epi8(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_mul_epi8(__W);
}

char test_mm256_reduce_and_epi8(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_and_epi8(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_and_epi8(__W);
}

char test_mm256_reduce_or_epi8(__m256i __W){
// CHECK-LABEL: @test_mm256_reduce_or_epi8(
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_reduce_or_epi8(__W);
}

char test_mm256_mask_reduce_add_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_add_epi8(
// CHECK:    bitcast i32 %{{.*}} to <32 x i1>
// CHECK:    select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    add <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_add_epi8(__M, __W);
}

char test_mm256_mask_reduce_mul_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_mul_epi8(
// CHECK:    bitcast i32 %{{.*}} to <32 x i1>
// CHECK:    select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    mul <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_mul_epi8(__M, __W);
}

char test_mm256_mask_reduce_and_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_and_epi8(
// CHECK:    bitcast i32 %{{.*}} to <32 x i1>
// CHECK:    select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    and <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_and_epi8(__M, __W);
}

char test_mm256_mask_reduce_or_epi8(__mmask32 __M, __m256i __W){
// CHECK-LABEL: @test_mm256_mask_reduce_or_epi8(
// CHECK:    bitcast i32 %{{.*}} to <32 x i1>
// CHECK:    select <32 x i1> %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}}
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 0, i32 1>
// CHECK:    shufflevector <4 x i64> %{{.*}}, <4 x i64> undef, <2 x i32> <i32 2, i32 3>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 2, i32 3, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    shufflevector <16 x i8> %{{.*}}, <16 x i8> %{{.*}}, <16 x i32> <i32 1, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
// CHECK:    or <16 x i8> %{{.*}}, %{{.*}}
// CHECK:    extractelement <16 x i8> %{{.*}}, i32 0
  return _mm256_mask_reduce_or_epi8(__M, __W);
}
