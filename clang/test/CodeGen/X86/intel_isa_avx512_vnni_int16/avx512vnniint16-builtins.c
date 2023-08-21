// REQUIRES: intel_feature_isa_avx512_vnni_int16
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512vnniint16 \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512i test_mm512_dpwsud_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwsud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwsud_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwsud_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwsud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwsud_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwsud_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwsud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwsud_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_dpwsuds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwsuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwsuds_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwsuds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwsuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwsuds_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwsuds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwsuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwsuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwsuds_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_dpwusd_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwusd_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusd.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwusd_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwusd_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwusd_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusd.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwusd_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwusd_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwusd_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusd.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwusd_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_dpwusds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwusds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwusds_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwusds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwusds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwusds_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwusds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwusds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwusds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwusds_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_dpwuud_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwuud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwuud_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwuud_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwuud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwuud_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwuud_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwuud_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuud.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwuud_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_dpwuuds_epi32(__m512i __A, __m512i __B, __m512i __C) {
  // CHECK-LABEL: @test_mm512_dpwuuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  return _mm512_dpwuuds_epi32(__A, __B, __C);
}

__m512i test_mm512_mask_dpwuuds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_mask_dpwuuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_mask_dpwuuds_epi32(__A, __B, __C, __D);
}

__m512i test_mm512_maskz_dpwuuds_epi32(__m512i __A, __mmask16 __B, __m512i __C, __m512i __D) {
  // CHECK-LABEL: @test_mm512_maskz_dpwuuds_epi32(
  // CHECK: call <16 x i32> @llvm.x86.avx2.vpdpwuuds.512(<16 x i32> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}})
  // CHECK: select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
  return _mm512_maskz_dpwuuds_epi32(__A, __B, __C, __D);
}
