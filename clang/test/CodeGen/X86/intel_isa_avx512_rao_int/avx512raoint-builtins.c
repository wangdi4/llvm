// REQUIRES: intel_feature_isa_avx512_rao_int
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512raoint \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm512_vpaadd_epi32(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaadd_epi32(
  // CHECK: call void @llvm.x86.vpaaddd512(i8* %{{.*}}, <16 x i32> %{{.*}})
  _mm512_vpaadd_epi32(__A, __B);
}

void test_mm512_mask_vpaadd_epi32(__m512i *__A, __mmask16 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaadd_epi32(
  // CHECK: call void @llvm.x86.mask.vpaaddd512(i8* %{{.*}}, <16 x i32> %{{.*}}, i16 %{{.*}})
  _mm512_mask_vpaadd_epi32(__A, __B, __C);
}

void test_mm512_vpaadd_epi64(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaadd_epi64(
  // CHECK: call void @llvm.x86.vpaaddq512(i8* %{{.*}}, <8 x i64> %{{.*}})
  _mm512_vpaadd_epi64(__A, __B);
}

void test_mm512_mask_vpaadd_epi64(__m512i *__A, __mmask8 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaadd_epi64(
  // CHECK: call void @llvm.x86.mask.vpaaddq512(i8* %{{.*}}, <8 x i64> %{{.*}}, i8 %{{.*}})
  _mm512_mask_vpaadd_epi64(__A, __B, __C);
}

void test_mm512_vpaand_epi32(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaand_epi32(
  // CHECK: call void @llvm.x86.vpaandd512(i8* %{{.*}}, <16 x i32> %{{.*}})
  _mm512_vpaand_epi32(__A, __B);
}

void test_mm512_mask_vpaand_epi32(__m512i *__A, __mmask16 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaand_epi32(
  // CHECK: call void @llvm.x86.mask.vpaandd512(i8* %{{.*}}, <16 x i32> %{{.*}}, i16 %{{.*}})
  _mm512_mask_vpaand_epi32(__A, __B, __C);
}

void test_mm512_vpaand_epi64(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaand_epi64(
  // CHECK: call void @llvm.x86.vpaandq512(i8* %{{.*}}, <8 x i64> %{{.*}})
  _mm512_vpaand_epi64(__A, __B);
}

void test_mm512_mask_vpaand_epi64(__m512i *__A, __mmask8 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaand_epi64(
  // CHECK: call void @llvm.x86.mask.vpaandq512(i8* %{{.*}}, <8 x i64> %{{.*}}, i8 %{{.*}})
  _mm512_mask_vpaand_epi64(__A, __B, __C);
}

void test_mm512_vpaor_epi32(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaor_epi32(
  // CHECK: call void @llvm.x86.vpaord512(i8* %{{.*}}, <16 x i32> %{{.*}})
  _mm512_vpaor_epi32(__A, __B);
}

void test_mm512_mask_vpaor_epi32(__m512i *__A, __mmask16 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaord512(i8* %{{.*}}, <16 x i32> %{{.*}}, i16 %{{.*}})
  _mm512_mask_vpaor_epi32(__A, __B, __C);
}

void test_mm512_vpaor_epi64(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaor_epi64(
  // CHECK: call void @llvm.x86.vpaorq512(i8* %{{.*}}, <8 x i64> %{{.*}})
  _mm512_vpaor_epi64(__A, __B);
}

void test_mm512_mask_vpaor_epi64(__m512i *__A, __mmask8 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaorq512(i8* %{{.*}}, <8 x i64> %{{.*}}, i8 %{{.*}})
  _mm512_mask_vpaor_epi64(__A, __B, __C);
}

void test_mm512_vpaxor_epi32(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaxor_epi32(
  // CHECK: call void @llvm.x86.vpaxord512(i8* %{{.*}}, <16 x i32> %{{.*}})
  _mm512_vpaxor_epi32(__A, __B);
}

void test_mm512_mask_vpaxor_epi32(__m512i *__A, __mmask16 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaxor_epi32(
  // CHECK: call void @llvm.x86.mask.vpaxord512(i8* %{{.*}}, <16 x i32> %{{.*}}, i16 %{{.*}})
  _mm512_mask_vpaxor_epi32(__A, __B, __C);
}

void test_mm512_vpaxor_epi64(__m512i *__A, __m512i __B) {
  // CHECK-LABEL: @test_mm512_vpaxor_epi64(
  // CHECK: call void @llvm.x86.vpaxorq512(i8* %{{.*}}, <8 x i64> %{{.*}})
  _mm512_vpaxor_epi64(__A, __B);
}

void test_mm512_mask_vpaxor_epi64(__m512i *__A, __mmask8 __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_mask_vpaxor_epi64(
  // CHECK: call void @llvm.x86.mask.vpaxorq512(i8* %{{.*}}, <8 x i64> %{{.*}}, i8 %{{.*}})
  _mm512_mask_vpaxor_epi64(__A, __B, __C);
}
