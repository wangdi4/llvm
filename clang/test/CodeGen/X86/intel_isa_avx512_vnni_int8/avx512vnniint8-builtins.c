// REQUIRES: intel_feature_isa_avx512_vnni_int8
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-unknown-unknown -target-feature +avx512vnniint8 -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

// CHECK-LABEL: @test_mm512_dpbssd_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssd.512
__m512i test_mm512_dpbssd_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbssd_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbssd_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssd.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbssd_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbssd_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbssd_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssd.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbssd_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbssd_epi32(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_dpbssds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssds.512
__m512i test_mm512_dpbssds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbssds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbssds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbssds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbssds_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbssds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbssds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbssds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbssds_epi32(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_dpbsud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsud.512
__m512i test_mm512_dpbsud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbsud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbsud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsud.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbsud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbsud_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbsud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsud.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbsud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbsud_epi32(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_dpbsuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsuds.512
__m512i test_mm512_dpbsuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbsuds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbsuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsuds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbsuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbsuds_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbsuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbsuds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbsuds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbsuds_epi32(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_dpbuud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuud.512
__m512i test_mm512_dpbuud_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbuud_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbuud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuud.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbuud_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbuud_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbuud_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuud.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbuud_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbuud_epi32(__U, __W, __A, __B);
}

// CHECK-LABEL: @test_mm512_dpbuuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuuds.512
__m512i test_mm512_dpbuuds_epi32(__m512i __W, __m512i __A, __m512i __B) {
  return _mm512_dpbuuds_epi32(__W, __A, __B);
}

// CHECK-LABEL: @test_mm512_mask_dpbuuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuuds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_mask_dpbuuds_epi32(__m512i __W, __mmask16 __U, __m512i __A, __m512i __B) {
  return _mm512_mask_dpbuuds_epi32(__W, __U,  __A, __B);
}

// CHECK-LABEL: @test_mm512_maskz_dpbuuds_epi32(
// CHECK:     call <16 x i32> @llvm.x86.avx512.vpdpbuuds.512
// CHECK:     select <16 x i1> %{{.*}}, <16 x i32> %{{.*}}, <16 x i32> %{{.*}}
__m512i test_mm512_maskz_dpbuuds_epi32(__mmask16 __U, __m512i __W, __m512i __A, __m512i __B) {
  return _mm512_maskz_dpbuuds_epi32(__U, __W, __A, __B);
}
