// REQUIRES: intel_feature_isa_vpinsr_vpextr
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +vpinsr-vpextr \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>

int test_mm256_extract_epi8(__m256i A) {
  // CHECK-LABEL: test_mm256_extract_epi8
  // CHECK: extractelement <32 x i8> %{{.*}}, {{i32|i64}} 31
  // CHECK: zext i8 %{{.*}} to i32
  return _mm256_extract_epi8(A, 31);
}

int test_mm256_extract_epi16(__m256i A) {
  // CHECK-LABEL: test_mm256_extract_epi16
  // CHECK: extractelement <16 x i16> %{{.*}}, {{i32|i64}} 15
  // CHECK: zext i16 %{{.*}} to i32
  return _mm256_extract_epi16(A, 15);
}

int test_mm256_extract_epi32(__m256i A) {
  // CHECK-LABEL: test_mm256_extract_epi32
  // CHECK: extractelement <8 x i32> %{{.*}}, {{i32|i64}} 7
  return _mm256_extract_epi32(A, 7);
}

#if __x86_64__
long long test_mm256_extract_epi64(__m256i A) {
  // CHECK-LABEL: test_mm256_extract_epi64
  // CHECK: extractelement <4 x i64> %{{.*}}, {{i32|i64}} 3
  return _mm256_extract_epi64(A, 3);
}
#endif

int test_mm512_extract_epi8(__m512i A) {
  // CHECK-LABEL: test_mm512_extract_epi8
  // CHECK: extractelement <64 x i8> %{{.*}}, {{i32|i64}} 63
  // CHECK: zext i8 %{{.*}} to i32
  return _mm512_extract_epi8(A, 63);
}

int test_mm512_extract_epi16(__m512i A) {
  // CHECK-LABEL: test_mm512_extract_epi16
  // CHECK: extractelement <32 x i16> %{{.*}}, {{i32|i64}} 31
  // CHECK: zext i16 %{{.*}} to i32
  return _mm512_extract_epi16(A, 31);
}

int test_mm512_extract_epi32(__m512i A) {
  // CHECK-LABEL: test_mm512_extract_epi32
  // CHECK: extractelement <16 x i32> %{{.*}}, {{i32|i64}} 15
  return _mm512_extract_epi32(A, 15);
}

#if __x86_64__
long long test_mm512_extract_epi64(__m512i A) {
  // CHECK-LABEL: test_mm512_extract_epi64
  // CHECK: extractelement <8 x i64> %{{.*}}, {{i32|i64}} 7
  return _mm512_extract_epi64(A, 7);
}
#endif

__m256i test_mm256_insert_epi8(__m256i x, char b) {
  // CHECK-LABEL: test_mm256_insert_epi8
  // CHECK: insertelement <32 x i8> %{{.*}}, i8 %{{.*}}, {{i32|i64}} 14
  return _mm256_insert_epi8(x, b, 14);
}

__m256i test_mm256_insert_epi16(__m256i x, int b) {
  // CHECK-LABEL: test_mm256_insert_epi16
  // CHECK: insertelement <16 x i16> %{{.*}}, i16 %{{.*}}, {{i32|i64}} 4
  return _mm256_insert_epi16(x, b, 4);
}

__m256i test_mm256_insert_epi32(__m256i x, int b) {
  // CHECK-LABEL: test_mm256_insert_epi32
  // CHECK: insertelement <8 x i32> %{{.*}}, i32 %{{.*}}, {{i32|i64}} 5
  return _mm256_insert_epi32(x, b, 5);
}

#if __x86_64__
__m256i test_mm256_insert_epi64(__m256i x, long long b) {
  // CHECK-LABEL: test_mm256_insert_epi64
  // CHECK: insertelement <4 x i64> %{{.*}}, i64 %{{.*}}, {{i32|i64}} 2
  return _mm256_insert_epi64(x, b, 2);
}
#endif

__m512i test_mm512_insert_epi8(__m512i x, char b) {
  // CHECK-LABEL: test_mm512_insert_epi8
  // CHECK: insertelement <64 x i8> %{{.*}}, i8 %{{.*}}, {{i32|i64}} 63
  return _mm512_insert_epi8(x, b, 63);
}

__m512i test_mm512_insert_epi16(__m512i x, int b) {
  // CHECK-LABEL: test_mm512_insert_epi16
  // CHECK: insertelement <32 x i16> %{{.*}}, i16 %{{.*}}, {{i32|i64}} 31
  return _mm512_insert_epi16(x, b, 31);
}

__m512i test_mm512_insert_epi32(__m512i x, int b) {
  // CHECK-LABEL: test_mm512_insert_epi32
  // CHECK: insertelement <16 x i32> %{{.*}}, i32 %{{.*}}, {{i32|i64}} 15
  return _mm512_insert_epi32(x, b, 15);
}

#if __x86_64__
__m512i test_mm512_insert_epi64(__m512i x, long long b) {
  // CHECK-LABEL: test_mm512_insert_epi64
  // CHECK: insertelement <8 x i64> %{{.*}}, i64 %{{.*}}, {{i32|i64}} 7
  return _mm512_insert_epi64(x, b, 7);
}
#endif
