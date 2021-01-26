// REQUIRES: intel_feature_isa_avx_rao_int
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx2 -target-feature +avxraoint \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm_vpaaddd_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaaddd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaaddd_epi32(__A, __B);
}

void test_mm256_vpaaddd_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaaddd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaaddd_epi32(__A, __B);
}

void test_mm_vpaaddq_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaaddq_epi64(
  // CHECK: call void @llvm.x86.vpaaddq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaaddq_epi64(__A, __B);
}

void test_mm256_vpaaddq_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaaddq_epi64(
  // CHECK: call void @llvm.x86.vpaaddq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaaddq_epi64(__A, __B);
}

void test_mm_vpaandd_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaandd_epi32(
  // CHECK: call void @llvm.x86.vpaandd128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaandd_epi32(__A, __B);
}

void test_mm256_vpaandd_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaandd_epi32(
  // CHECK: call void @llvm.x86.vpaandd256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaandd_epi32(__A, __B);
}

void test_mm_vpaandq_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaandq_epi64(
  // CHECK: call void @llvm.x86.vpaandq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaandq_epi64(__A, __B);
}

void test_mm256_vpaandq_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaandq_epi64(
  // CHECK: call void @llvm.x86.vpaandq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaandq_epi64(__A, __B);
}

void test_mm_vpaord_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaord_epi32(
  // CHECK: call void @llvm.x86.vpaord128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaord_epi32(__A, __B);
}

void test_mm256_vpaord_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaord_epi32(
  // CHECK: call void @llvm.x86.vpaord256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaord_epi32(__A, __B);
}

void test_mm_vpaorq_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaorq_epi64(
  // CHECK: call void @llvm.x86.vpaorq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaorq_epi64(__A, __B);
}

void test_mm256_vpaorq_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaorq_epi64(
  // CHECK: call void @llvm.x86.vpaorq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaorq_epi64(__A, __B);
}

void test_mm_vpaxord_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxord_epi32(
  // CHECK: call void @llvm.x86.vpaxord128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaxord_epi32(__A, __B);
}

void test_mm256_vpaxord_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxord_epi32(
  // CHECK: call void @llvm.x86.vpaxord256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaxord_epi32(__A, __B);
}

void test_mm_vpaxorq_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxorq_epi64(
  // CHECK: call void @llvm.x86.vpaxorq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaxorq_epi64(__A, __B);
}

void test_mm256_vpaxorq_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxorq_epi64(
  // CHECK: call void @llvm.x86.vpaxorq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaxorq_epi64(__A, __B);
}
