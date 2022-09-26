// REQUIRES: intel_feature_isa_avx512_rao_fp
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512raofp -target-feature +avx512vl \
// RUN: -target-feature +avx512fp16 -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm_vaadd_pbh(__m128bh *__A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_vaadd_pbh(
  // CHECK: call void @llvm.x86.vaaddpbf16128(i8* %{{.*}}, <8 x i16> %{{.*}})
  _mm_vaadd_pbh(__A, __B);
}

void test_mm_mask_vaadd_pbh(__m128bh *__A, __mmask8 __B, __m128bh __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_pbh(
  // CHECK: call void @llvm.x86.mask.vaaddpbf16128(i8* %{{.*}}, <8 x i16> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_pbh(__A, __B, __C);
}

void test_mm256_vaadd_pbh(__m256bh *__A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_vaadd_pbh(
  // CHECK: call void @llvm.x86.vaaddpbf16256(i8* %{{.*}}, <16 x i16> %{{.*}})
  _mm256_vaadd_pbh(__A, __B);
}

void test_mm256_mask_vaadd_pbh(__m256bh *__A, __mmask16 __B, __m256bh __C) {
  // CHECK-LABEL: @test_mm256_mask_vaadd_pbh(
  // CHECK: call void @llvm.x86.mask.vaaddpbf16256(i8* %{{.*}}, <16 x i16> %{{.*}}, i16 %{{.*}})
  _mm256_mask_vaadd_pbh(__A, __B, __C);
}

void test_mm_vaadd_pd(__m128d *__A, __m128d __B) {
  // CHECK-LABEL: @test_mm_vaadd_pd(
  // CHECK: call void @llvm.x86.vaaddpd128(i8* %{{.*}}, <2 x double> %{{.*}})
  _mm_vaadd_pd(__A, __B);
}

void test_mm_mask_vaadd_pd(__m128d *__A, __mmask8 __B, __m128d __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_pd(
  // CHECK: call void @llvm.x86.mask.vaaddpd128(i8* %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_pd(__A, __B, __C);
}

void test_mm256_vaadd_pd(__m256d *__A, __m256d __B) {
  // CHECK-LABEL: @test_mm256_vaadd_pd(
  // CHECK: call void @llvm.x86.vaaddpd256(i8* %{{.*}}, <4 x double> %{{.*}})
  _mm256_vaadd_pd(__A, __B);
}

void test_mm256_mask_vaadd_pd(__m256d *__A, __mmask8 __B, __m256d __C) {
  // CHECK-LABEL: @test_mm256_mask_vaadd_pd(
  // CHECK: call void @llvm.x86.mask.vaaddpd256(i8* %{{.*}}, <4 x double> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vaadd_pd(__A, __B, __C);
}

void test_mm_vaadd_ph(__m128h *__A, __m128h __B) {
  // CHECK-LABEL: @test_mm_vaadd_ph(
  // CHECK: call void @llvm.x86.vaaddph128(i8* %{{.*}}, <8 x half> %{{.*}})
  _mm_vaadd_ph(__A, __B);
}

void test_mm_mask_vaadd_ph(__m128h *__A, __mmask8 __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_ph(
  // CHECK: call void @llvm.x86.mask.vaaddph128(i8* %{{.*}}, <8 x half> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_ph(__A, __B, __C);
}

void test_mm256_vaadd_ph(__m256h *__A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_vaadd_ph(
  // CHECK: call void @llvm.x86.vaaddph256(i8* %{{.*}}, <16 x half> %{{.*}})
  _mm256_vaadd_ph(__A, __B);
}

void test_mm256_mask_vaadd_ph(__m256h *__A, __mmask16 __B, __m256h __C) {
  // CHECK-LABEL: @test_mm256_mask_vaadd_ph(
  // CHECK: call void @llvm.x86.mask.vaaddph256(i8* %{{.*}}, <16 x half> %{{.*}}, i16 %{{.*}})
  _mm256_mask_vaadd_ph(__A, __B, __C);
}

void test_mm_vaadd_ps(__m128 *__A, __m128 __B) {
  // CHECK-LABEL: @test_mm_vaadd_ps(
  // CHECK: call void @llvm.x86.vaaddps128(i8* %{{.*}}, <4 x float> %{{.*}})
  _mm_vaadd_ps(__A, __B);
}

void test_mm_mask_vaadd_ps(__m128 *__A, __mmask8 __B, __m128 __C) {
  // CHECK-LABEL: @test_mm_mask_vaadd_ps(
  // CHECK: call void @llvm.x86.mask.vaaddps128(i8* %{{.*}}, <4 x float> %{{.*}}, i8 %{{.*}})
  _mm_mask_vaadd_ps(__A, __B, __C);
}

void test_mm256_vaadd_ps(__m256 *__A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_vaadd_ps(
  // CHECK: call void @llvm.x86.vaaddps256(i8* %{{.*}}, <8 x float> %{{.*}})
  _mm256_vaadd_ps(__A, __B);
}

void test_mm256_mask_vaadd_ps(__m256 *__A, __mmask8 __B, __m256 __C) {
  // CHECK-LABEL: @test_mm256_mask_vaadd_ps(
  // CHECK: call void @llvm.x86.mask.vaaddps256(i8* %{{.*}}, <8 x float> %{{.*}}, i8 %{{.*}})
  _mm256_mask_vaadd_ps(__A, __B, __C);
}
