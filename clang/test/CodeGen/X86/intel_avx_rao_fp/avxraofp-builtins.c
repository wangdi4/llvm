// REQUIRES: intel_feature_isa_avx_rao_fp
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx2 -target-feature +avxraofp -target-feature +avx512fp16 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

void test_mm_vaadd_pbh(__m128bh *__A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_vaadd_pbh(
  // CHECK: call void @llvm.x86.vaaddpbf16128(i8* %{{.*}}, <8 x i16> %{{.*}})
  _mm_vaadd_pbh(__A, __B);
}

void test_mm256_vaadd_pbh(__m256bh *__A, __m256bh __B) {
  // CHECK-LABEL: @test_mm256_vaadd_pbh(
  // CHECK: call void @llvm.x86.vaaddpbf16256(i8* %{{.*}}, <16 x i16> %{{.*}})
  _mm256_vaadd_pbh(__A, __B);
}

void test_mm_vaadd_pd(__m128d *__A, __m128d __B) {
  // CHECK-LABEL: @test_mm_vaadd_pd(
  // CHECK: call void @llvm.x86.vaaddpd128(i8* %{{.*}}, <2 x double> %{{.*}})
  _mm_vaadd_pd(__A, __B);
}

void test_mm256_vaadd_pd(__m256d *__A, __m256d __B) {
  // CHECK-LABEL: @test_mm256_vaadd_pd(
  // CHECK: call void @llvm.x86.vaaddpd256(i8* %{{.*}}, <4 x double> %{{.*}})
  _mm256_vaadd_pd(__A, __B);
}

void test_mm_vaadd_ph(__m128h *__A, __m128h __B) {
  // CHECK-LABEL: @test_mm_vaadd_ph(
  // CHECK: call void @llvm.x86.vaaddph128(i8* %{{.*}}, <8 x half> %{{.*}})
  _mm_vaadd_ph(__A, __B);
}

void test_mm256_vaadd_ph(__m256h *__A, __m256h __B) {
  // CHECK-LABEL: @test_mm256_vaadd_ph(
  // CHECK: call void @llvm.x86.vaaddph256(i8* %{{.*}}, <16 x half> %{{.*}})
  _mm256_vaadd_ph(__A, __B);
}

void test_mm_vaadd_ps(__m128 *__A, __m128 __B) {
  // CHECK-LABEL: @test_mm_vaadd_ps(
  // CHECK: call void @llvm.x86.vaaddps128(i8* %{{.*}}, <4 x float> %{{.*}})
  _mm_vaadd_ps(__A, __B);
}

void test_mm256_vaadd_ps(__m256 *__A, __m256 __B) {
  // CHECK-LABEL: @test_mm256_vaadd_ps(
  // CHECK: call void @llvm.x86.vaaddps256(i8* %{{.*}}, <8 x float> %{{.*}})
  _mm256_vaadd_ps(__A, __B);
}

void test_mm_vaadd_sbh(void *__A, __m128bh __B) {
  // CHECK-LABEL: @test_mm_vaadd_sbh(
  // CHECK: call void @llvm.x86.vaaddsbf16128(i8* %{{.*}}, <8 x i16> %{{.*}})
  _mm_vaadd_sbh(__A, __B);
}

void test_mm_vaadd_sd(void *__A, __m128d __B) {
  // CHECK-LABEL: @test_mm_vaadd_sd(
  // CHECK: call void @llvm.x86.vaaddsd128(i8* %{{.*}}, <2 x double> %{{.*}})
  _mm_vaadd_sd(__A, __B);
}

void test_mm_vaadd_sh(void *__A, __m128h __B) {
  // CHECK-LABEL: @test_mm_vaadd_sh(
  // CHECK: call void @llvm.x86.vaaddsh128(i8* %{{.*}}, <8 x half> %{{.*}})
  _mm_vaadd_sh(__A, __B);
}

void test_mm_vaadd_ss(void *__A, __m128 __B) {
  // CHECK-LABEL: @test_mm_vaadd_ss(
  // CHECK: call void @llvm.x86.vaaddss128(i8* %{{.*}}, <4 x float> %{{.*}})
  _mm_vaadd_ss(__A, __B);
}
