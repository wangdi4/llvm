// REQUIRES: intel_feature_isa_avx512_minmax
// RUN: %clang_cc1 %s -flax-vector-conversions=none -ffreestanding -triple=x86_64-unknown-unknown -target-feature +avx512f -target-feature +avx512minmax \
// RUN: -emit-llvm -o - -Wall -Werror -pedantic -Wno-gnu-statement-expression | FileCheck %s

#include <immintrin.h>
#include <stddef.h>

__m512bh test_mm512_minmaxne_pbh(__m512bh __A, __m512bh __B) {
  // CHECK-LABEL: @test_mm512_minmaxne_pbh(
  // CHECK: call <32 x i16> @llvm.x86.avx512minmax.vminmaxnepbf16512(
  return _mm512_minmaxne_pbh(__A, __B, 127);
}

__m512bh test_mm512_mask_minmaxne_pbh(__m512bh __A, __mmask32 __B, __m512bh __C, __m512bh __D) {
  // CHECK-LABEL: @test_mm512_mask_minmaxne_pbh(
  // CHECK: call <32 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16512(
  return _mm512_mask_minmaxne_pbh(__A, __B, __C, __D, 127);
}

__m512bh test_mm512_maskz_minmaxne_pbh(__mmask32 __A, __m512bh __B, __m512bh __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmaxne_pbh(
  // CHECK: call <32 x i16> @llvm.x86.avx512minmax.mask.vminmaxnepbf16512(
  return _mm512_maskz_minmaxne_pbh(__A, __B, __C, 127);
}

__m512d test_mm512_minmax_pd(__m512d __A, __m512d __B) {
  // CHECK-LABEL: @test_mm512_minmax_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_minmax_pd(__A, __B, 127);
}

__m512d test_mm512_mask_minmax_pd(__m512d __A, __mmask8 __B, __m512d __C, __m512d __D) {
  // CHECK-LABEL: @test_mm512_mask_minmax_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_mask_minmax_pd(__A, __B, __C, __D, 127);
}

__m512d test_mm512_maskz_minmax_pd(__mmask8 __A, __m512d __B, __m512d __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmax_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_maskz_minmax_pd(__A, __B, __C, 127);
}

__m512d test_mm512_minmaxpd_round_pd(__m512d __A, __m512d __B) {
  // CHECK-LABEL: @test_mm512_minmaxpd_round_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_minmaxpd_round_pd(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m512d test_mm512_mask_minmaxpd_round_pd(__m512d __A, __mmask8 __B, __m512d __C, __m512d __D) {
  // CHECK-LABEL: @test_mm512_mask_minmaxpd_round_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_mask_minmaxpd_round_pd(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m512d test_mm512_maskz_minmaxpd_round_pd(__mmask8 __A, __m512d __B, __m512d __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmaxpd_round_pd(
  // CHECK: call <8 x double> @llvm.x86.avx512minmax.mask.vminmaxpd.round(
  return _mm512_maskz_minmaxpd_round_pd(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

__m512h test_mm512_minmax_ph(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_minmax_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_minmax_ph(__A, __B, 127);
}

__m512h test_mm512_mask_minmax_ph(__m512h __A, __mmask32 __B, __m512h __C, __m512h __D) {
  // CHECK-LABEL: @test_mm512_mask_minmax_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_mask_minmax_ph(__A, __B, __C, __D, 127);
}

__m512h test_mm512_maskz_minmax_ph(__mmask32 __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmax_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_maskz_minmax_ph(__A, __B, __C, 127);
}

__m512h test_mm512_minmaxph_round_ph(__m512h __A, __m512h __B) {
  // CHECK-LABEL: @test_mm512_minmaxph_round_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_minmaxph_round_ph(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m512h test_mm512_mask_minmaxph_round_ph(__m512h __A, __mmask32 __B, __m512h __C, __m512h __D) {
  // CHECK-LABEL: @test_mm512_mask_minmaxph_round_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_mask_minmaxph_round_ph(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m512h test_mm512_maskz_minmaxph_round_ph(__mmask32 __A, __m512h __B, __m512h __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmaxph_round_ph(
  // CHECK: call <32 x half> @llvm.x86.avx512minmax.mask.vminmaxph.round(
  return _mm512_maskz_minmaxph_round_ph(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

__m512 test_mm512_minmax_ps(__m512 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_minmax_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_minmax_ps(__A, __B, 127);
}

__m512 test_mm512_mask_minmax_ps(__m512 __A, __mmask16 __B, __m512 __C, __m512 __D) {
  // CHECK-LABEL: @test_mm512_mask_minmax_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_mask_minmax_ps(__A, __B, __C, __D, 127);
}

__m512 test_mm512_maskz_minmax_ps(__mmask16 __A, __m512 __B, __m512 __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmax_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_maskz_minmax_ps(__A, __B, __C, 127);
}

__m512 test_mm512_minmaxps_round_ps(__m512 __A, __m512 __B) {
  // CHECK-LABEL: @test_mm512_minmaxps_round_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_minmaxps_round_ps(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m512 test_mm512_mask_minmaxps_round_ps(__m512 __A, __mmask16 __B, __m512 __C, __m512 __D) {
  // CHECK-LABEL: @test_mm512_mask_minmaxps_round_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_mask_minmaxps_round_ps(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m512 test_mm512_maskz_minmaxps_round_ps(__mmask16 __A, __m512 __B, __m512 __C) {
  // CHECK-LABEL: @test_mm512_maskz_minmaxps_round_ps(
  // CHECK: call <16 x float> @llvm.x86.avx512minmax.mask.vminmaxps.round(
  return _mm512_maskz_minmaxps_round_ps(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

__m128d test_mm_minmaxsd_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_minmaxsd_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_minmaxsd_sd(__A, __B, 127);
}

__m128d test_mm_mask_minmaxsd_sd(__m128d __A, __mmask8 __B, __m128d __C, __m128d __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxsd_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_mask_minmaxsd_sd(__A, __B, __C, __D, 127);
}

__m128d test_mm_maskz_minmaxsd_sd(__mmask8 __A, __m128d __B, __m128d __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxsd_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_maskz_minmaxsd_sd(__A, __B, __C, 127);
}

__m128d test_mm_minmaxsd_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_minmaxsd_round_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_minmaxsd_round_sd(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_minmaxsd_round_sd(__m128d __A, __mmask8 __B, __m128d __C, __m128d __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxsd_round_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_mask_minmaxsd_round_sd(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_minmaxsd_round_sd(__mmask8 __A, __m128d __B, __m128d __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxsd_round_sd(
  // CHECK: call <2 x double> @llvm.x86.avx512minmax.mask.vminmaxsd.round(
  return _mm_maskz_minmaxsd_round_sd(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

__m128h test_mm_minmaxsh_sh(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_minmaxsh_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_minmaxsh_sh(__A, __B, 127);
}

__m128h test_mm_mask_minmaxsh_sh(__m128h __A, __mmask8 __B, __m128h __C, __m128h __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxsh_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_mask_minmaxsh_sh(__A, __B, __C, __D, 127);
}

__m128h test_mm_maskz_minmaxsh_sh(__mmask8 __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxsh_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_maskz_minmaxsh_sh(__A, __B, __C, 127);
}

__m128h test_mm_minmaxsh_round_sh(__m128h __A, __m128h __B) {
  // CHECK-LABEL: @test_mm_minmaxsh_round_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_minmaxsh_round_sh(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m128h test_mm_mask_minmaxsh_round_sh(__m128h __A, __mmask8 __B, __m128h __C, __m128h __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxsh_round_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_mask_minmaxsh_round_sh(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m128h test_mm_maskz_minmaxsh_round_sh(__mmask8 __A, __m128h __B, __m128h __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxsh_round_sh(
  // CHECK: call <8 x half> @llvm.x86.avx512minmax.mask.vminmaxsh.round(
  return _mm_maskz_minmaxsh_round_sh(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

__m128 test_mm_minmaxss_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_minmaxss_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_minmaxss_ss(__A, __B, 127);
}

__m128 test_mm_mask_minmaxss_ss(__m128 __A, __mmask8 __B, __m128 __C, __m128 __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxss_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_mask_minmaxss_ss(__A, __B, __C, __D, 127);
}

__m128 test_mm_maskz_minmaxss_ss(__mmask8 __A, __m128 __B, __m128 __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxss_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_maskz_minmaxss_ss(__A, __B, __C, 127);
}

__m128 test_mm_minmaxss_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_minmaxss_round_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_minmaxss_round_ss(__A, __B, 127, _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_minmaxss_round_ss(__m128 __A, __mmask8 __B, __m128 __C, __m128 __D) {
  // CHECK-LABEL: @test_mm_mask_minmaxss_round_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_mask_minmaxss_round_ss(__A, __B, __C, __D, 127, _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_minmaxss_round_ss(__mmask8 __A, __m128 __B, __m128 __C) {
  // CHECK-LABEL: @test_mm_maskz_minmaxss_round_ss(
  // CHECK: call <4 x float> @llvm.x86.avx512minmax.mask.vminmaxss.round(
  return _mm_maskz_minmaxss_round_ss(__A, __B, __C, 127, _MM_FROUND_NO_EXC);
}

