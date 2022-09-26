// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512fp16 -target-feature +avx512raofp \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm512_vaadd_pbh(__m512bh *__A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_vaadd_pbh(
  // CHECK: call void @llvm.x86.vaaddpbf16512(i8* %{{.*}}, <32 x i16> %{{.*}})
  _mm512_vaadd_pbh(__A, __B);
}

void test_mm512_mask_vaadd_pbh(__m512bh *__A, __mmask32 __B, __m512bh __C) {
  // CHECK-LABEL: @test_mm512_mask_vaadd_pbh(
  // CHECK: call void @llvm.x86.mask.vaaddpbf16512(i8* %{{.*}}, <32 x i16> %{{.*}}, i32 %{{.*}})
  _mm512_mask_vaadd_pbh(__A, __B, __C);
}

void test_mm512_vaadd_pd(__m512d *__A, __m512d __B) {
  // CHECK-LABEL: @test_mm512_vaadd_pd(
  // CHECK: call void @llvm.x86.vaaddpd512(i8* %{{.*}}, <8 x double> %{{.*}})
  _mm512_vaadd_pd(__A, __B);
}

void test_mm512_mask_vaadd_pd(__m512d *__A, __mmask8 __B, __m512d __C) {
  // CHECK-LABEL: @test_mm512_mask_vaadd_pd(
  // CHECK: call void @llvm.x86.mask.vaaddpd512(i8* %{{.*}}, <8 x double> %{{.*}}, i8 %{{.*}})
  _mm512_mask_vaadd_pd(__A, __B, __C);
}

void test_mm512_vaadd_ph(__m512h *__A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_vaadd_ph(
  // CHECK: call void @llvm.x86.vaaddph512(i8* %{{.*}}, <32 x half> %{{.*}})
  _mm512_vaadd_ph(__A, __B);
}

void test_mm512_mask_vaadd_ph(__m512h *__A, __mmask32 __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_mask_vaadd_ph(
  // CHECK: call void @llvm.x86.mask.vaaddph512(i8* %{{.*}}, <32 x half> %{{.*}}, i32 %{{.*}})
  _mm512_mask_vaadd_ph(__A, __B, __C);
}

void test_mm512_vaadd_ps(__m512 *__A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_vaadd_ps(
  // CHECK: call void @llvm.x86.vaaddps512(i8* %{{.*}}, <16 x float> %{{.*}})
  _mm512_vaadd_ps(__A, __B);
}

void test_mm512_mask_vaadd_ps(__m512 *__A, __mmask16 __B, __m512 __C) {
  // CHECK-LABEL: @test_mm512_mask_vaadd_ps(
  // CHECK: call void @llvm.x86.mask.vaaddps512(i8* %{{.*}}, <16 x float> %{{.*}}, i16 %{{.*}})
  _mm512_mask_vaadd_ps(__A, __B, __C);
}

void test_mm_vaadd_sbh(void *__A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_vaadd_sbh(
  // CHECK: call void @llvm.x86.vaaddsbf16128(i8* %{{.*}}, <8 x i16> %{{.*}})
  _mm_vaadd_sbh(__A, __B);
}

void test_mm_mask_vaadd_sbh(void *__A, __mmask8 __B, __m128bh __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_sbh(
  // CHECK: call void @llvm.x86.mask.vaaddsbf16128(i8* %{{.*}}, <8 x i16> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_sbh(__A, __B, __C);
}

void test_mm_vaadd_sd(void *__A, __m128d __B) {
  // CHECK-LABEL: @test_mm_vaadd_sd(
  // CHECK: call void @llvm.x86.vaaddsd128(i8* %{{.*}}, <2 x double> %{{.*}})
  _mm_vaadd_sd(__A, __B);
}

void test_mm_mask_vaadd_sd(void *__A, __mmask8 __B, __m128d __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_sd(
  // CHECK: call void @llvm.x86.mask.vaaddsd128(i8* %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_sd(__A, __B, __C);
}

void test_mm_vaadd_sh(void *__A, __m128h __B) {
  // CHECK-LABEL: @test_mm_vaadd_sh(
  // CHECK: call void @llvm.x86.vaaddsh128(i8* %{{.*}}, <8 x half> %{{.*}})
  _mm_vaadd_sh(__A, __B);
}

void test_mm_mask_vaadd_sh(void *__A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_sh(
  // CHECK: call void @llvm.x86.mask.vaaddsh128(i8* %{{.*}}, <8 x half> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_sh(__A, __B, __C);
}

void test_mm_vaadd_ss(void *__A, __m128 __B) {
  // CHECK-LABEL: @test_mm_vaadd_ss(
  // CHECK: call void @llvm.x86.vaaddss128(i8* %{{.*}}, <4 x float> %{{.*}})
  _mm_vaadd_ss(__A, __B);
}

void test_mm_mask_vaadd_ss(void *__A, __mmask8 __B, __m128 __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_ss(
  // CHECK: call void @llvm.x86.mask.vaaddss128(i8* %{{.*}}, <4 x float> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_ss(__A, __B, __C);
}
