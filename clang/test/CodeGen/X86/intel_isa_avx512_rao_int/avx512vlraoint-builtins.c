// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512raoint -target-feature +avx512vl \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm_vpaadd_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaadd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaadd_epi32(__A, __B);
}

void test_mm_mask_vpaadd_epi32(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaadd_epi32(
  // CHECK: call void @llvm.x86.mask.vpaaddd128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaadd_epi32(__A, __B, __C);
}

void test_mm256_vpaadd_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaadd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaadd_epi32(__A, __B);
}

void test_mm256_mask_vpaadd_epi32(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaadd_epi32(
  // CHECK: call void @llvm.x86.mask.vpaaddd256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaadd_epi32(__A, __B, __C);
}

void test_mm_vpaadd_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaadd_epi64(
  // CHECK: call void @llvm.x86.vpaaddq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaadd_epi64(__A, __B);
}

void test_mm_mask_vpaadd_epi64(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaadd_epi64(
  // CHECK: call void @llvm.x86.mask.vpaaddq128(i8* %{{.*}}, <2 x i64> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaadd_epi64(__A, __B, __C);
}

void test_mm256_vpaadd_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaadd_epi64(
  // CHECK: call void @llvm.x86.vpaaddq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaadd_epi64(__A, __B);
}

void test_mm256_mask_vpaadd_epi64(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaadd_epi64(
  // CHECK: call void @llvm.x86.mask.vpaaddq256(i8* %{{.*}}, <4 x i64> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaadd_epi64(__A, __B, __C);
}

void test_mm_vpaand_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaand_epi32(
  // CHECK: call void @llvm.x86.vpaandd128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaand_epi32(__A, __B);
}

void test_mm_mask_vpaand_epi32(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaand_epi32(
  // CHECK: call void @llvm.x86.mask.vpaandd128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaand_epi32(__A, __B, __C);
}

void test_mm256_vpaand_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaand_epi32(
  // CHECK: call void @llvm.x86.vpaandd256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaand_epi32(__A, __B);
}

void test_mm256_mask_vpaand_epi32(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaand_epi32(
  // CHECK: call void @llvm.x86.mask.vpaandd256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaand_epi32(__A, __B, __C);
}

void test_mm_vpaand_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaand_epi64(
  // CHECK: call void @llvm.x86.vpaandq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaand_epi64(__A, __B);
}

void test_mm_mask_vpaand_epi64(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaand_epi64(
  // CHECK: call void @llvm.x86.mask.vpaandq128(i8* %{{.*}}, <2 x i64> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaand_epi64(__A, __B, __C);
}

void test_mm256_vpaand_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaand_epi64(
  // CHECK: call void @llvm.x86.vpaandq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaand_epi64(__A, __B);
}

void test_mm256_mask_vpaand_epi64(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaand_epi64(
  // CHECK: call void @llvm.x86.mask.vpaandq256(i8* %{{.*}}, <4 x i64> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaand_epi64(__A, __B, __C);
}

void test_mm_vpaor_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaor_epi32(
  // CHECK: call void @llvm.x86.vpaord128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaor_epi32(__A, __B);
}

void test_mm_mask_vpaor_epi32(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaord128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaor_epi32(__A, __B, __C);
}

void test_mm256_vpaor_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaor_epi32(
  // CHECK: call void @llvm.x86.vpaord256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaor_epi32(__A, __B);
}

void test_mm256_mask_vpaor_epi32(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaord256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaor_epi32(__A, __B, __C);
}

void test_mm_vpaor_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaor_epi64(
  // CHECK: call void @llvm.x86.vpaorq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaor_epi64(__A, __B);
}

void test_mm_mask_vpaor_epi64(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaorq128(i8* %{{.*}}, <2 x i64> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaor_epi64(__A, __B, __C);
}

void test_mm256_vpaor_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaor_epi64(
  // CHECK: call void @llvm.x86.vpaorq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaor_epi64(__A, __B);
}

void test_mm256_mask_vpaor_epi64(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaorq256(i8* %{{.*}}, <4 x i64> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaor_epi64(__A, __B, __C);
}

void test_mm_vpaxor_epi32(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxor_epi32(
  // CHECK: call void @llvm.x86.vpaxord128(i8* %{{.*}}, <4 x i32> %{{.*}})
  _mm_vpaxor_epi32(__A, __B);
}

void test_mm_mask_vpaxor_epi32(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaxor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaxord128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaxor_epi32(__A, __B, __C);
}

void test_mm256_vpaxor_epi32(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxor_epi32(
  // CHECK: call void @llvm.x86.vpaxord256(i8* %{{.*}}, <8 x i32> %{{.*}})
  _mm256_vpaxor_epi32(__A, __B);
}

void test_mm256_mask_vpaxor_epi32(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaxor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaxord256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaxor_epi32(__A, __B, __C);
}

void test_mm_vpaxor_epi64(__m128i *__A, __m128i __B) {
  // CHECK-LABEL: @test_mm_vpaxor_epi64(
  // CHECK: call void @llvm.x86.vpaxorq128(i8* %{{.*}}, <2 x i64> %{{.*}})
  _mm_vpaxor_epi64(__A, __B);
}

void test_mm_mask_vpaxor_epi64(__m128i *__A, __mmask8 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_vpaxor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaxorq128(i8* %{{.*}}, <2 x i64> %{{.*}}, i8 %{{.*}})
  _mm_mask_vpaxor_epi64(__A, __B, __C);
}

void test_mm256_vpaxor_epi64(__m256i *__A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_vpaxor_epi64(
  // CHECK: call void @llvm.x86.vpaxorq256(i8* %{{.*}}, <4 x i64> %{{.*}})
  _mm256_vpaxor_epi64(__A, __B);
}

void test_mm256_mask_vpaxor_epi64(__m256i *__A, __mmask8 __B, __m256i __C) {
  // CHECK-LABEL: @test_mm256_mask_vpaxor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaxorq256(i8* %{{.*}}, <4 x i64> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vpaxor_epi64(__A, __B, __C);
}
