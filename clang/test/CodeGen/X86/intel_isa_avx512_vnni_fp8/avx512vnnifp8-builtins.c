// REQUIRES: intel_feature_isa_avx512_vnni_fp8
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512vnnifp8 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512 test_mm512_dpbf8_ps(__m512 __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps512(
  return _mm512_dpbf8_ps(__A, __B, __C);
}

__m512 test_mm512_mask_dpbf8_ps(__m512 __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps512(
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_mask_dpbf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_maskz_dpbf8_ps(__mmask16 __A, __m512 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbf8ps512(
  // CHECK: zeroinitializer
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_maskz_dpbf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_dpbhf8_ps(__m512 __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpbhf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps512(
  return _mm512_dpbhf8_ps(__A, __B, __C);
}

__m512 test_mm512_mask_dpbhf8_ps(__m512 __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpbhf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps512(
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_mask_dpbhf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_maskz_dpbhf8_ps(__mmask16 __A, __m512 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpbhf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdpbhf8ps512(
  // CHECK: zeroinitializer
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_maskz_dpbhf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_dphbf8_ps(__m512 __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dphbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps512(
  return _mm512_dphbf8_ps(__A, __B, __C);
}

__m512 test_mm512_mask_dphbf8_ps(__m512 __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dphbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps512(
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_mask_dphbf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_maskz_dphbf8_ps(__mmask16 __A, __m512 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dphbf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphbf8ps512(
  // CHECK: zeroinitializer
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_maskz_dphbf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_dphf8_ps(__m512 __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dphf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphf8ps512(
  return _mm512_dphf8_ps(__A, __B, __C);
}

__m512 test_mm512_mask_dphf8_ps(__m512 __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dphf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphf8ps512(
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_mask_dphf8_ps(__A, __B, __C, __D);
}

__m512 test_mm512_maskz_dphf8_ps(__mmask16 __A, __m512 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dphf8_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512vnnifp8.vdphf8ps512(
  // CHECK: zeroinitializer
  // CHECK: select <16 x i1> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}
  return _mm512_maskz_dphf8_ps(__A, __B, __C, __D);
}

