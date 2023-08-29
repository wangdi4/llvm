// REQUIRES: intel_feature_isa_avx_rao_int
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx2 -target-feature +avxraoint \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm_vpaadd_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaadd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd128(ptr %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaadd_epi32(__A, __B);
}

void test_mm256_vpaadd_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaadd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd256(ptr %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaadd_epi32(__A, __B);
}

void test_mm_vpaadd_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaadd_epi64(
  // CHECK: call void @llvm.x86.vpaaddq128(ptr %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaadd_epi64(__A, __B);
}

void test_mm256_vpaadd_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaadd_epi64(
  // CHECK: call void @llvm.x86.vpaaddq256(ptr %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaadd_epi64(__A, __B);
}

void test_mm_vpaand_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaand_epi32(
  // CHECK: call void @llvm.x86.vpaandd128(ptr %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaand_epi32(__A, __B);
}

void test_mm256_vpaand_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaand_epi32(
  // CHECK: call void @llvm.x86.vpaandd256(ptr %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaand_epi32(__A, __B);
}

void test_mm_vpaand_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaand_epi64(
  // CHECK: call void @llvm.x86.vpaandq128(ptr %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaand_epi64(__A, __B);
}

void test_mm256_vpaand_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaand_epi64(
  // CHECK: call void @llvm.x86.vpaandq256(ptr %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaand_epi64(__A, __B);
}

void test_mm_vpaor_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaor_epi32(
  // CHECK: call void @llvm.x86.vpaord128(ptr %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaor_epi32(__A, __B);
}

void test_mm256_vpaor_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaor_epi32(
  // CHECK: call void @llvm.x86.vpaord256(ptr %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaor_epi32(__A, __B);
}

void test_mm_vpaor_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaor_epi64(
  // CHECK: call void @llvm.x86.vpaorq128(ptr %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaor_epi64(__A, __B);
}

void test_mm256_vpaor_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaor_epi64(
  // CHECK: call void @llvm.x86.vpaorq256(ptr %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaor_epi64(__A, __B);
}

void test_mm_vpaxor_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxor_epi32(
  // CHECK: call void @llvm.x86.vpaxord128(ptr %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaxor_epi32(__A, __B);
}

void test_mm256_vpaxor_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxor_epi32(
  // CHECK: call void @llvm.x86.vpaxord256(ptr %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaxor_epi32(__A, __B);
}

void test_mm_vpaxor_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxor_epi64(
  // CHECK: call void @llvm.x86.vpaxorq128(ptr %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaxor_epi64(__A, __B);
}

void test_mm256_vpaxor_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxor_epi64(
  // CHECK: call void @llvm.x86.vpaxorq256(ptr %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaxor_epi64(__A, __B);
}
