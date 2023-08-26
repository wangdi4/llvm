// REQUIRES: intel_feature_isa_avx512_bf16_ne
// RUN: %clang_cc1 -ffreestanding -flax-vector-conversions=none %s -triple=x86_64-unknown-unknown -target-feature +avx512vl -target-feature +avx512bf16ne -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m256bf16 test_mm256_setzero_pbf16() {
  // CHECK-LABEL: @test_mm256_setzero_pbf16
  // CHECK: zeroinitializer
  return _mm256_setzero_pbf16();
}

__m128bf16 test_mm_setzero_pbf16() {
  // CHECK-LABEL: @test_mm_setzero_pbf16
  // CHECK: zeroinitializer
  return _mm_setzero_pbf16();
}

__m256bf16 test_mm256_undefined_pbf16(void) {
  // CHECK-LABEL: @test_mm256_undefined_pbf16
  // CHECK: ret <16 x bfloat> zeroinitializer
  return _mm256_undefined_pbf16();
}

__m128bf16 test_mm_undefined_pbf16(void) {
  // CHECK-LABEL: @test_mm_undefined_pbf16
  // CHECK: ret <8 x bfloat> zeroinitializer
  return _mm_undefined_pbf16();
}

__bf16 test_mm_cvtsbf16_bf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_cvtsbf16_bf16
  // CHECK: extractelement <8 x bfloat> %{{.*}}, i32 0
  return _mm_cvtsbf16_bf16(__A);
}

__bf16 test_mm256_cvtsbf16_bf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_cvtsbf16_bf16
  // CHECK: extractelement <16 x bfloat> %{{.*}}, i32 0
  return _mm256_cvtsbf16_bf16(__A);
}

__m128bf16 test_mm_set_sbf16(__bf16 h) {
  // CHECK-LABEL: @test_mm_set_sbf16
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 1
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 2
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 3
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 4
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 5
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 6
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 7
  return _mm_set_sbf16(h);
}

__m128bf16 test_mm_set1_pbf16(__bf16 h) {
  // CHECK-LABEL: @test_mm_set1_pbf16
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 7
  return _mm_set1_pbf16(h);
}

__m256bf16 test_mm256_set1_pbf16(__bf16 h) {
  // CHECK-LABEL: @test_mm256_set1_pbf16
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 15
  return _mm256_set1_pbf16(h);
}

__m128bf16 test_mm_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                       __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8) {
  // CHECK-LABEL: @test_mm_set_pbf16
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 7
  return _mm_set_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8);
}

__m256bf16 test_mm256_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                          __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8,
                          __bf16 bf9, __bf16 bf10, __bf16 bf11, __bf16 bf12,
                          __bf16 bf13, __bf16 bf14, __bf16 bf15, __bf16 bf16) {
  // CHECK-LABEL: @test_mm256_set_pbf16
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 15
  return _mm256_set_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8,
                       bf9, bf10, bf11, bf12, bf13, bf14, bf15, bf16);
}

__m128bf16 test_mm_setr_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                        __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8) {
  // CHECK-LABEL: @test_mm_setr_pbf16
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <8 x bfloat> {{.*}}, i32 7
  return _mm_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8);
}

__m256bf16 test_mm256_setr_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                           __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8,
                           __bf16 bf9, __bf16 bf10, __bf16 bf11, __bf16 bf12,
                           __bf16 bf13, __bf16 bf14, __bf16 bf15, __bf16 bf16) {
  // CHECK-LABEL: @test_mm256_setr_pbf16
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <16 x bfloat> {{.*}}, i32 15
  return _mm256_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8,
                        bf9, bf10, bf11, bf12, bf13, bf14, bf15, bf16);
}

__m128bf16 test_mm_abs_pbf16(__m128bf16 a) {
  // CHECK-LABEL: @test_mm_abs_pbf16
  // CHECK: and <4 x i32>
  return _mm_abs_pbf16(a);
}

__m256bf16 test_mm256_abs_pbf16(__m256bf16 a) {
  // CHECK-LABEL: @test_mm256_abs_pbf16
  // CHECK: and <8 x i32>
  return _mm256_abs_pbf16(a);
}

__m128bf16 test_mm_mask_blend_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __W) {
  // CHECK-LABEL: @test_mm_mask_blend_pbf16
  // CHECK:  %{{.*}} = bitcast i8 %{{.*}} to <8 x i1>
  // CHECK:  %{{.*}} = select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask_blend_pbf16(__U, __A, __W);
}

__m256bf16 test_mm256_mask_blend_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __W) {
  // CHECK-LABEL: @test_mm256_mask_blend_pbf16
  // CHECK:  %{{.*}} = bitcast i16 %{{.*}} to <16 x i1>
  // CHECK:  %{{.*}} = select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask_blend_pbf16(__U, __A, __W);
}

__m128bf16 test_mm_permutex2var_pbf16(__m128bf16 __A, __m128i __I, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_permutex2var_pbf16
  // CHECK:  %{{.*}} = bitcast <8 x bfloat> %{{.*}} to <8 x i16>
  // CHECK:  %{{.*}} = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK:  %{{.*}} = bitcast <8 x bfloat> %{{.*}} to <8 x i16>
  // CHECK:  %{{.*}} = call <8 x i16> @llvm.x86.avx512.vpermi2var.hi.128(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <8 x i16> %{{.*}} to <8 x bfloat>
  return _mm_permutex2var_pbf16(__A, __I, __B);
}

__m256bf16 test_mm256_permutex2var_pbf16(__m256bf16 __A, __m256i __I, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_permutex2var_pbf16
  // CHECK:  %{{.*}} = bitcast <16 x bfloat> %{{.*}} to <16 x i16>
  // CHECK:  %{{.*}} = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK:  %{{.*}} = bitcast <16 x bfloat> %{{.*}} to <16 x i16>
  // CHECK:  %{{.*}} = call <16 x i16> @llvm.x86.avx512.vpermi2var.hi.256(<16 x i16> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <16 x i16> %{{.*}} to <16 x bfloat>
  return _mm256_permutex2var_pbf16(__A, __I, __B);
}

__m128bf16 test_mm_permutexvar_pbf16(__m128i __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_permutexvar_pbf16
  // CHECK:  %{{.*}} = bitcast <8 x bfloat> %{{.*}} to <8 x i16>
  // CHECK:  %{{.*}} = bitcast <2 x i64> %{{.*}} to <8 x i16>
  // CHECK:  %{{.*}} = call <8 x i16> @llvm.x86.avx512.permvar.hi.128(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <8 x i16> %{{.*}} to <8 x bfloat>
  return _mm_permutexvar_pbf16(__A, __B);
}

__m256bf16 test_mm256_permutexvar_pbf16(__m256i __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_permutexvar_pbf16
  // CHECK:  %{{.*}} = bitcast <16 x bfloat> %{{.*}} to <16 x i16>
  // CHECK:  %{{.*}} = bitcast <4 x i64> %{{.*}} to <16 x i16>
  // CHECK:  %{{.*}} = call <16 x i16> @llvm.x86.avx512.permvar.hi.256(<16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <16 x i16> %{{.*}} to <16 x bfloat>
  return _mm256_permutexvar_pbf16(__A, __B);
}

__m256bf16 test_mm256_addne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_addne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16256(
  return _mm256_addne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_addne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_addne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_addne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_addne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_addne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_addne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16128(
  return _mm_addne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_addne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_addne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_addne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_addne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_subne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_subne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16256(
  return _mm256_subne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_subne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_subne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_subne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_subne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_subne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_subne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16128(
  return _mm_subne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_subne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_subne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_subne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_subne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_mulne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_mulne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16256(
  return _mm256_mulne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_mulne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_mulne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_mulne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_mulne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_mulne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_mulne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16128(
  return _mm_mulne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_mulne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_mulne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_mulne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_mulne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_divne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_divne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16256(
  return _mm256_divne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_divne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_divne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_divne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_divne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_divne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_divne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16128(
  return _mm_divne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_divne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_divne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_divne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_divne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_maxne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_maxne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16256(
  return _mm256_maxne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_maxne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_maxne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_maxne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_maxne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_maxne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_maxne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16128(
  return _mm_maxne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_maxne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_maxne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_maxne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_maxne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_minne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_minne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16256(
  return _mm256_minne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_minne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return (__m256bf16)_mm256_mask_minne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_minne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16256
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_minne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_minne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_minne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16128(
  return _mm_minne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_minne_pbf16(__m128bf16 __W, __mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return (__m128bf16)_mm_mask_minne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_minne_pbf16(__mmask16 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16128
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_minne_pbf16(__U, __A, __B);
}

__mmask16 test_mm256_cmpne_pbf16_mask_eq_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: @test_mm256_cmpne_pbf16_mask_eq_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 0, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_EQ_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_lt_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_lt_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 1, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_LT_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_le_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_le_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 2, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_LE_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_unord_q(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_unord_q
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 3, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_UNORD_Q);
}

__mmask16 test_mm256_cmpne_pbf16_mask_neq_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_neq_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 4, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NEQ_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nlt_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nlt_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 5, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NLT_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nle_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nle_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 6, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NLE_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ord_q(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ord_q
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 7, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_ORD_Q);
}

__mmask16 test_mm256_cmpne_pbf16_mask_eq_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_eq_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 8, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_EQ_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nge_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nge_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 9, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NGE_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ngt_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ngt_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 10, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NGT_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_false_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_false_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 11, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_FALSE_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_neq_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_neq_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 12, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NEQ_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ge_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ge_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 13, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_GE_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_gt_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_gt_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 14, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_GT_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_true_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_true_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 15, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_TRUE_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_eq_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_eq_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 16, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_EQ_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_lt_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_lt_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 17, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_LT_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_le_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_le_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 18, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_LE_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_unord_s(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_unord_s
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 19, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_UNORD_S);
}

__mmask16 test_mm256_cmpne_pbf16_mask_neq_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_neq_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 20, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NEQ_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nlt_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 21, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NLT_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nle_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nle_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 22, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NLE_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ord_s(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ord_s
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 23, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_ORD_S);
}

__mmask16 test_mm256_cmpne_pbf16_mask_eq_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_eq_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 24, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_EQ_US);
}

__mmask16 test_mm256_cmpne_pbf16_mask_nge_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_nge_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 25, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NGE_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ngt_uq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 26, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NGT_UQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_false_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_false_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 27, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_FALSE_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_neq_os(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_neq_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 28, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_NEQ_OS);
}

__mmask16 test_mm256_cmpne_pbf16_mask_ge_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_ge_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 29, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_GE_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_gt_oq(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_gt_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 30, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_GT_OQ);
}

__mmask16 test_mm256_cmpne_pbf16_mask_true_us(__m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_cmpne_pbf16_mask_true_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 31, i16 -1)
  return _mm256_cmpne_pbf16_mask(a, b, _CMP_TRUE_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_eq_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: @test_mm256_mask_cmpne_pbf16_mask_eq_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 0, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_lt_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_lt_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 1, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_le_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_le_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 2, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_unord_q(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_unord_q
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 3, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_Q);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_neq_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_neq_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 4, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nlt_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nlt_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 5, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nle_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nle_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 6, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ord_q(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ord_q
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 7, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_Q);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_eq_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_eq_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 8, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nge_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nge_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 9, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ngt_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ngt_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 10, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_false_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_false_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 11, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_neq_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_neq_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 12, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ge_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ge_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 13, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_gt_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_gt_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 14, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_true_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_true_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 15, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_eq_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_eq_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 16, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_lt_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_lt_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 17, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_le_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_le_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 18, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_unord_s(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_unord_s
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 19, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_S);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_neq_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_neq_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 20, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nlt_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 21, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nle_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nle_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 22, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ord_s(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ord_s
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 23, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_S);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_eq_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_eq_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 24, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_US);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_nge_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_nge_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 25, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ngt_uq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 26, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_UQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_false_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_false_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 27, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_neq_os(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_neq_os
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 28, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OS);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_ge_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_ge_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 29, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_gt_oq(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_gt_oq
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 30, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OQ);
}

__mmask16 test_mm256_mask_cmpne_pbf16_mask_true_us(__mmask16 m, __m256bf16 a, __m256bf16 b) {
  // CHECK-LABEL: test_mm256_mask_cmpne_pbf16_mask_true_us
  // CHECK: call i16 @llvm.x86.avx512bf16ne.vcmpnepbf16256.mask(<16 x bfloat> %{{.}}, <16 x bfloat> %{{.}}, i32 31, i16 %{{.}})
  return _mm256_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_eq_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: @test_mm_cmpne_pbf16_mask_eq_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 0, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_EQ_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_lt_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_lt_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 1, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_LT_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_le_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_le_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 2, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_LE_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_unord_q(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_unord_q
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 3, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_UNORD_Q);
}

__mmask8 test_mm_cmpne_pbf16_mask_neq_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_neq_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 4, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NEQ_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_nlt_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nlt_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 5, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NLT_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_nle_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nle_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 6, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NLE_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_ord_q(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ord_q
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 7, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_ORD_Q);
}

__mmask8 test_mm_cmpne_pbf16_mask_eq_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_eq_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 8, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_EQ_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_nge_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nge_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 9, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NGE_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_ngt_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ngt_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 10, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NGT_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_false_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_false_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 11, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_FALSE_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_neq_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_neq_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 12, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NEQ_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_ge_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ge_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 13, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_GE_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_gt_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_gt_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 14, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_GT_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_true_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_true_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 15, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_TRUE_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_eq_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_eq_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 16, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_EQ_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_lt_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_lt_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 17, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_LT_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_le_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_le_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 18, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_LE_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_unord_s(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_unord_s
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 19, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_UNORD_S);
}

__mmask8 test_mm_cmpne_pbf16_mask_neq_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_neq_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 20, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NEQ_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_nlt_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 21, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NLT_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_nle_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nle_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 22, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NLE_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_ord_s(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ord_s
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 23, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_ORD_S);
}

__mmask8 test_mm_cmpne_pbf16_mask_eq_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_eq_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 24, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_EQ_US);
}

__mmask8 test_mm_cmpne_pbf16_mask_nge_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_nge_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 25, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NGE_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_ngt_uq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 26, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NGT_UQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_false_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_false_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 27, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_FALSE_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_neq_os(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_neq_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 28, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_NEQ_OS);
}

__mmask8 test_mm_cmpne_pbf16_mask_ge_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_ge_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 29, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_GE_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_gt_oq(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_gt_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 30, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_GT_OQ);
}

__mmask8 test_mm_cmpne_pbf16_mask_true_us(__m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_cmpne_pbf16_mask_true_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 31, i8 -1)
  return _mm_cmpne_pbf16_mask(a, b, _CMP_TRUE_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_eq_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: @test_mm_mask_cmpne_pbf16_mask_eq_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 0, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_lt_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_lt_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 1, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_le_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_le_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 2, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_unord_q(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_unord_q
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 3, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_Q);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_neq_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_neq_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 4, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nlt_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nlt_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 5, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nle_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nle_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 6, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ord_q(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ord_q
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 7, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_Q);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_eq_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_eq_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 8, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nge_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nge_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 9, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ngt_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ngt_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 10, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_false_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_false_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 11, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_neq_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_neq_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 12, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ge_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ge_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 13, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_gt_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_gt_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 14, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_true_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_true_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 15, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_eq_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_eq_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 16, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_lt_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_lt_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 17, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_le_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_le_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 18, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_unord_s(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_unord_s
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 19, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_S);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_neq_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_neq_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 20, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nlt_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 21, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nle_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nle_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 22, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ord_s(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ord_s
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 23, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_S);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_eq_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_eq_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 24, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_US);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_nge_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_nge_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 25, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ngt_uq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 26, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_UQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_false_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_false_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 27, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_neq_os(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_neq_os
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 28, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OS);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_ge_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_ge_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 29, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_gt_oq(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_gt_oq
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 30, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OQ);
}

__mmask8 test_mm_mask_cmpne_pbf16_mask_true_us(__mmask8 m, __m128bf16 a, __m128bf16 b) {
  // CHECK-LABEL: test_mm_mask_cmpne_pbf16_mask_true_us
  // CHECK: call i8 @llvm.x86.avx512bf16ne.vcmpnepbf16128.mask(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}}, i32 31, i8 %{{.}})
  return _mm_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_US);
}


__mmask16 test_mm256_mask_fpclassne_pbf16_mask(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.256
  return _mm256_mask_fpclassne_pbf16_mask(__U, __A, 4);
}

__mmask16 test_mm256_fpclassne_pbf16_mask(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.256
  return _mm256_fpclassne_pbf16_mask(__A, 4);
}

__mmask8 test_mm_mask_fpclassne_pbf16_mask(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.128
  return _mm_mask_fpclassne_pbf16_mask(__U, __A, 4);
}

__mmask8 test_mm_fpclassne_pbf16_mask(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.128
  return _mm_fpclassne_pbf16_mask(__A, 4);
}

__m256bf16 test_mm256_scalefne_pbf16(__m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.256
  return _mm256_scalefne_pbf16(__A, __B);
}

__m256bf16 test_mm256_mask_scalefne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_mask_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.256
  return _mm256_mask_scalefne_pbf16(__W, __U, __A, __B);
}

__m256bf16 test_mm256_maskz_scalefne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B) {
  // CHECK-LABEL: @test_mm256_maskz_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.256
  return _mm256_maskz_scalefne_pbf16(__U, __A, __B);
}

__m256bf16 test_mm256_rcpne_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.256
  return _mm256_rcpne_pbf16(__A);
}

__m256bf16 test_mm256_mask_rcpne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.256
  return (__m256bf16)_mm256_mask_rcpne_pbf16(__W, __U, __A);
}

__m256bf16 test_mm256_maskz_rcpne_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.256
  return _mm256_maskz_rcpne_pbf16(__U, __A);
}

__m256bf16 test_mm256_getexpne_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.256
  return _mm256_getexpne_pbf16(__A);
}

__m256bf16 test_mm256_mask_getexpne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.256
  return _mm256_mask_getexpne_pbf16(__W, __U, __A);
}

__m256bf16 test_mm256_maskz_getexpne_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.256
  return _mm256_maskz_getexpne_pbf16(__U, __A);
}

__m256bf16 test_mm256_rsqrtne_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.256
  return _mm256_rsqrtne_pbf16(__A);
}

__m256bf16 test_mm256_mask_rsqrtne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.256
  return (__m256bf16)_mm256_mask_rsqrtne_pbf16(__W, __U, __A);
}

__m256bf16 test_mm256_maskz_rsqrtne_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.256
  return _mm256_maskz_rsqrtne_pbf16(__U, __A);
}

__m256bf16 test_mm256_reducene_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.256
  return _mm256_reducene_pbf16(__A, 3);
}

__m256bf16 test_mm256_mask_reducene_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.256
  return _mm256_mask_reducene_pbf16(__W, __U, __A, 1);
}

__m256bf16 test_mm256_maskz_reducene_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.256
  return _mm256_maskz_reducene_pbf16(__U, __A, 1);
}

__m256bf16 test_mm256_roundscalene_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.256
  return _mm256_roundscalene_pbf16(__A, 3);
}

__m256bf16 test_mm256_mask_roundscalene_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.256
  return _mm256_mask_roundscalene_pbf16(__W, __U, __A, 1);
}

__m256bf16 test_mm256_maskz_roundscalene_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.256
  return _mm256_maskz_roundscalene_pbf16(__U, __A, 1 );
}

__m256bf16 test_mm256_getmantne_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.256
  return _mm256_getmantne_pbf16(__A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m256bf16 test_mm256_mask_getmantne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.256
  return _mm256_mask_getmantne_pbf16(__W, __U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m256bf16 test_mm256_maskz_getmantne_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.256
  return _mm256_maskz_getmantne_pbf16(__U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m256bf16 test_mm256_sqrtne_pbf16(__m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.256
  return _mm256_sqrtne_pbf16(__A);
}

__m256bf16 test_mm256_mask_sqrtne_pbf16(__m256bf16 __W, __mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_mask_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.256
  return (__m256bf16)_mm256_mask_sqrtne_pbf16(__W, __U, __A);
}

__m256bf16 test_mm256_maskz_sqrtne_pbf16(__mmask16 __U, __m256bf16 __A) {
  // CHECK-LABEL: @test_mm256_maskz_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.256
  return _mm256_maskz_sqrtne_pbf16(__U, __A);
}

__m128bf16 test_mm_scalefne_pbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.128
  return _mm_scalefne_pbf16(__A, __B);
}

__m128bf16 test_mm_mask_scalefne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_mask_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.128
  return _mm_mask_scalefne_pbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_scalefne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_maskz_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.128
  return _mm_maskz_scalefne_pbf16(__U, __A, __B);
}

__m128bf16 test_mm_rcpne_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.128
  return _mm_rcpne_pbf16(__A);
}

__m128bf16 test_mm_mask_rcpne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.128
  return (__m128bf16)_mm_mask_rcpne_pbf16(__W, __U, __A);
}

__m128bf16 test_mm_maskz_rcpne_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.128
  return _mm_maskz_rcpne_pbf16(__U, __A);
}

__m128bf16 test_mm_getexpne_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.128
  return _mm_getexpne_pbf16(__A);
}

__m128bf16 test_mm_mask_getexpne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.128
  return _mm_mask_getexpne_pbf16(__W, __U, __A);
}

__m128bf16 test_mm_maskz_getexpne_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.128
  return _mm_maskz_getexpne_pbf16(__U, __A);
}

__m128bf16 test_mm_rsqrtne_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.128
  return _mm_rsqrtne_pbf16(__A);
}

__m128bf16 test_mm_mask_rsqrtne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.128
  return (__m128bf16)_mm_mask_rsqrtne_pbf16(__W, __U, __A);
}

__m128bf16 test_mm_maskz_rsqrtne_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.128
  return _mm_maskz_rsqrtne_pbf16(__U, __A);
}

__m128bf16 test_mm_reducene_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.128
  return _mm_reducene_pbf16(__A, 3);
}

__m128bf16 test_mm_mask_reducene_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.128
  return _mm_mask_reducene_pbf16(__W, __U, __A, 1);
}

__m128bf16 test_mm_maskz_reducene_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.128
  return _mm_maskz_reducene_pbf16(__U, __A, 1);
}

__m128bf16 test_mm_roundscalene_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.128
  return _mm_roundscalene_pbf16(__A, 3);
}

__m128bf16 test_mm_mask_roundscalene_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.128
  return _mm_mask_roundscalene_pbf16(__W, __U, __A, 1);
}

__m128bf16 test_mm_maskz_roundscalene_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.128
  return _mm_maskz_roundscalene_pbf16(__U, __A, 1 );
}

__m128bf16 test_mm_getmantne_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.128
  return _mm_getmantne_pbf16(__A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128bf16 test_mm_mask_getmantne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.128
  return _mm_mask_getmantne_pbf16(__W, __U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128bf16 test_mm_maskz_getmantne_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.128
  return _mm_maskz_getmantne_pbf16(__U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128bf16 test_mm_sqrtne_pbf16(__m128bf16 __A) {
  // CHECK-LABEL: @test_mm_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.128
  return _mm_sqrtne_pbf16(__A);
}

__m128bf16 test_mm_mask_sqrtne_pbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.128
  return (__m128bf16)_mm_mask_sqrtne_pbf16(__W, __U, __A);
}

__m128bf16 test_mm_maskz_sqrtne_pbf16(__mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_maskz_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.128
  return _mm_maskz_sqrtne_pbf16(__U, __A);
}

__m256bf16 test_mm256_fmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16256(
  return _mm256_fmaddne_pbf16(__A, __B, __C);
}

__m256bf16 test_mm256_mask_fmaddne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_mask_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd132nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask_fmaddne_pbf16(__A, __U, __B, __C);
}

__m256bf16 test_mm256_mask3_fmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C, __mmask16 __U) {
  // CHECK-LABEL: @test_mm256_mask3_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd231nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask3_fmaddne_pbf16(__A, __B, __C, __U);
}

__m256bf16 test_mm256_maskz_fmaddne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_maskz_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_fmaddne_pbf16(__U, __A, __B, __C);
}

__m256bf16 test_mm256_fmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16256(
  return _mm256_fmsubne_pbf16(__A, __B, __C);
}

__m256bf16 test_mm256_mask_fmsubne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_mask_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub132nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask_fmsubne_pbf16(__A, __U, __B, __C);
}

__m256bf16 test_mm256_mask3_fmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C, __mmask16 __U) {
  // CHECK-LABEL: @test_mm256_mask3_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub231nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask3_fmsubne_pbf16(__A, __B, __C, __U);
}

__m256bf16 test_mm256_maskz_fmsubne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_maskz_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_fmsubne_pbf16(__U, __A, __B, __C);
}

__m256bf16 test_mm256_fnmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16256(
  return _mm256_fnmaddne_pbf16(__A, __B, __C);
}

__m256bf16 test_mm256_mask_fnmaddne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_mask_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd132nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask_fnmaddne_pbf16(__A, __U, __B, __C);
}

__m256bf16 test_mm256_mask3_fnmaddne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C, __mmask16 __U) {
  // CHECK-LABEL: @test_mm256_mask3_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd231nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask3_fnmaddne_pbf16(__A, __B, __C, __U);
}

__m256bf16 test_mm256_maskz_fnmaddne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_maskz_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_fnmaddne_pbf16(__U, __A, __B, __C);
}

__m256bf16 test_mm256_fnmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16256(
  return _mm256_fnmsubne_pbf16(__A, __B, __C);
}

__m256bf16 test_mm256_mask_fnmsubne_pbf16(__m256bf16 __A, __mmask16 __U, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_mask_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub132nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask_fnmsubne_pbf16(__A, __U, __B, __C);
}

__m256bf16 test_mm256_mask3_fnmsubne_pbf16(__m256bf16 __A, __m256bf16 __B, __m256bf16 __C, __mmask16 __U) {
  // CHECK-LABEL: @test_mm256_mask3_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub231nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_mask3_fnmsubne_pbf16(__A, __B, __C, __U);
}

__m256bf16 test_mm256_maskz_fnmsubne_pbf16(__mmask16 __U, __m256bf16 __A, __m256bf16 __B, __m256bf16 __C) {
  // CHECK-LABEL: @test_mm256_maskz_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16256(
  // CHECK: select <16 x i1> %{{.*}}, <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}
  return _mm256_maskz_fnmsubne_pbf16(__U, __A, __B, __C);
}

__m128bf16 test_mm_fmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16128(
  return _mm_fmaddne_pbf16(__A, __B, __C);
}

__m128bf16 test_mm_mask_fmaddne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_mask_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd132nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask_fmaddne_pbf16(__A, __U, __B, __C);
}

__m128bf16 test_mm_mask3_fmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C, __mmask8 __U) {
  // CHECK-LABEL: @test_mm_mask3_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd231nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask3_fmaddne_pbf16(__A, __B, __C, __U);
}

__m128bf16 test_mm_maskz_fmaddne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_maskz_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_fmaddne_pbf16(__U, __A, __B, __C);
}

__m128bf16 test_mm_fmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16128(
  return _mm_fmsubne_pbf16(__A, __B, __C);
}

__m128bf16 test_mm_mask_fmsubne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_mask_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub132nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask_fmsubne_pbf16(__A, __U, __B, __C);
}

__m128bf16 test_mm_mask3_fmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C, __mmask8 __U) {
  // CHECK-LABEL: @test_mm_mask3_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub231nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask3_fmsubne_pbf16(__A, __B, __C, __U);
}

__m128bf16 test_mm_maskz_fmsubne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_maskz_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_fmsubne_pbf16(__U, __A, __B, __C);
}

__m128bf16 test_mm_fnmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16128(
  return _mm_fnmaddne_pbf16(__A, __B, __C);
}

__m128bf16 test_mm_mask_fnmaddne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_mask_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd132nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask_fnmaddne_pbf16(__A, __U, __B, __C);
}

__m128bf16 test_mm_mask3_fnmaddne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C, __mmask8 __U) {
  // CHECK-LABEL: @test_mm_mask3_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd231nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask3_fnmaddne_pbf16(__A, __B, __C, __U);
}

__m128bf16 test_mm_maskz_fnmaddne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_maskz_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_fnmaddne_pbf16(__U, __A, __B, __C);
}

__m128bf16 test_mm_fnmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16128(
  return _mm_fnmsubne_pbf16(__A, __B, __C);
}

__m128bf16 test_mm_mask_fnmsubne_pbf16(__m128bf16 __A, __mmask8 __U, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_mask_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub132nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask_fnmsubne_pbf16(__A, __U, __B, __C);
}

__m128bf16 test_mm_mask3_fnmsubne_pbf16(__m128bf16 __A, __m128bf16 __B, __m128bf16 __C, __mmask8 __U) {
  // CHECK-LABEL: @test_mm_mask3_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub231nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_mask3_fnmsubne_pbf16(__A, __B, __C, __U);
}

__m128bf16 test_mm_maskz_fnmsubne_pbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B, __m128bf16 __C) {
  // CHECK-LABEL: @test_mm_maskz_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16128(
  // CHECK: select <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}
  return _mm_maskz_fnmsubne_pbf16(__U, __A, __B, __C);
}
