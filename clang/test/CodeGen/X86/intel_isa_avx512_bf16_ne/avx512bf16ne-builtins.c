// REQUIRES: intel_feature_isa_avx512_bf16_ne
// RUN: %clang_cc1 -ffreestanding -flax-vector-conversions=none %s -triple=x86_64-unknown-unknown -target-feature +avx512bf16ne -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m512bf16 test_mm512_setzero_pbf16() {
  // CHECK-LABEL: @test_mm512_setzero_pbf16
  // CHECK: zeroinitializer
  return _mm512_setzero_pbf16();
}

__m512bf16 test_mm512_undefined_pbf16(void) {
  // CHECK-LABEL: @test_mm512_undefined_pbf16
  // CHECK: ret <32 x bfloat> zeroinitializer
  return _mm512_undefined_pbf16();
}

__m512bf16 test_mm512_set1_pbf16(__bf16 h) {
  // CHECK-LABEL: @test_mm512_set1_pbf16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 15
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 17
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 18
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 19
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 20
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 21
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 22
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 23
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 24
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 25
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 26
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 27
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 28
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 29
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 30
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 31
  return _mm512_set1_pbf16(h);
}

__m512bf16 test_mm512_set_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                          __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8,
                          __bf16 bf9, __bf16 bf10, __bf16 bf11, __bf16 bf12,
                          __bf16 bf13, __bf16 bf14, __bf16 bf15, __bf16 bf16,
                          __bf16 bf17, __bf16 bf18, __bf16 bf19, __bf16 bf20,
                          __bf16 bf21, __bf16 bf22, __bf16 bf23, __bf16 bf24,
                          __bf16 bf25, __bf16 bf26, __bf16 bf27, __bf16 bf28,
                          __bf16 bf29, __bf16 bf30, __bf16 bf31, __bf16 bf32) {
  // CHECK-LABEL: @test_mm512_set_pbf16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 15
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 17
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 18
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 19
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 20
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 21
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 22
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 23
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 24
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 25
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 26
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 27
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 28
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 29
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 30
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 31
  return _mm512_set_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8,
                       bf9, bf10, bf11, bf12, bf13, bf14, bf15, bf16,
                       bf17, bf18, bf19, bf20, bf21, bf22, bf23, bf24,
                       bf25, bf26, bf27, bf28, bf29, bf30, bf31, bf32);
}

__m512bf16 test_mm512_setr_pbf16(__bf16 bf1, __bf16 bf2, __bf16 bf3, __bf16 bf4,
                           __bf16 bf5, __bf16 bf6, __bf16 bf7, __bf16 bf8,
                           __bf16 bf9, __bf16 bf10, __bf16 bf11, __bf16 bf12,
                           __bf16 bf13, __bf16 bf14, __bf16 bf15, __bf16 bf16,
                           __bf16 bf17, __bf16 bf18, __bf16 bf19, __bf16 bf20,
                           __bf16 bf21, __bf16 bf22, __bf16 bf23, __bf16 bf24,
                           __bf16 bf25, __bf16 bf26, __bf16 bf27, __bf16 bf28,
                           __bf16 bf29, __bf16 bf30, __bf16 bf31, __bf16 bf32) {
  // CHECK-LABEL: @test_mm512_setr_pbf16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 0
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 1
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 2
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 3
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 4
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 5
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 6
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 7
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 8
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 9
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 10
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 11
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 12
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 13
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 14
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 15
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 16
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 17
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 18
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 19
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 20
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 21
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 22
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 23
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 24
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 25
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 26
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 27
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 28
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 29
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 30
  // CHECK: insertelement <32 x bfloat> {{.*}}, i32 31
  return _mm512_setr_pbf16(bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8,
                        bf9, bf10, bf11, bf12, bf13, bf14, bf15, bf16,
                        bf17, bf18, bf19, bf20, bf21, bf22, bf23, bf24,
                        bf25, bf26, bf27, bf28, bf29, bf30, bf31, bf32);
}

__m128 test_mm_castpbf16_ps(__m128bf16 A) {
  // CHECK-LABEL: test_mm_castpbf16_ps
  // CHECK: bitcast <8 x bfloat> %{{.*}} to <4 x float>
  return _mm_castpbf16_ps(A);
}

__m256 test_mm256_castpbf16_ps(__m256bf16 A) {
  // CHECK-LABEL: test_mm256_castpbf16_ps
  // CHECK: bitcast <16 x bfloat> %{{.*}} to <8 x float>
  return _mm256_castpbf16_ps(A);
}

__m512 test_mm512_castpbf16_ps(__m512bf16 A) {
  // CHECK-LABEL: test_mm512_castpbf16_ps
  // CHECK: bitcast <32 x bfloat> %{{.*}} to <16 x float>
  return _mm512_castpbf16_ps(A);
}

__m128d test_mm_castpbf16_pd(__m128bf16 A) {
  // CHECK-LABEL: test_mm_castpbf16_pd
  // CHECK: bitcast <8 x bfloat> %{{.*}} to <2 x double>
  return _mm_castpbf16_pd(A);
}

__m256d test_mm256_castpbf16_pd(__m256bf16 A) {
  // CHECK-LABEL: test_mm256_castpbf16_pd
  // CHECK: bitcast <16 x bfloat> %{{.*}} to <4 x double>
  return _mm256_castpbf16_pd(A);
}

__m512d test_mm512_castpbf16_pd(__m512bf16 A) {
  // CHECK-LABEL: test_mm512_castpbf16_pd
  // CHECK: bitcast <32 x bfloat> %{{.*}} to <8 x double>
  return _mm512_castpbf16_pd(A);
}

__m128i test_mm_castpbf16_si128(__m128bf16 A) {
  // CHECK-LABEL: test_mm_castpbf16_si128
  // CHECK: bitcast <8 x bfloat> %{{.*}} to <2 x i64>
  return _mm_castpbf16_si128(A);
}

__m256i test_mm256_castpbf16_si256(__m256bf16 A) {
  // CHECK-LABEL: test_mm256_castpbf16_si256
  // CHECK: bitcast <16 x bfloat> %{{.*}} to <4 x i64>
  return _mm256_castpbf16_si256(A);
}

__m512i test_mm512_castpbf16_si512(__m512bf16 A) {
  // CHECK-LABEL: test_mm512_castpbf16_si512
  // CHECK: bitcast <32 x bfloat> %{{.*}} to <8 x i64>
  return _mm512_castpbf16_si512(A);
}

__m128bf16 test_mm_castps_pbf16(__m128 A) {
  // CHECK-LABEL: test_mm_castps_pbf16
  // CHECK: bitcast <4 x float> %{{.*}} to <8 x bfloat>
  return _mm_castps_pbf16(A);
}

__m256bf16 test_mm256_castps_pbf16(__m256 A) {
  // CHECK-LABEL: test_mm256_castps_pbf16
  // CHECK: bitcast <8 x float> %{{.*}} to <16 x bfloat>
  return _mm256_castps_pbf16(A);
}

__m512bf16 test_mm512_castps_pbf16(__m512 A) {
  // CHECK-LABEL: test_mm512_castps_pbf16
  // CHECK: bitcast <16 x float> %{{.*}} to <32 x bfloat>
  return _mm512_castps_pbf16(A);
}

__m128bf16 test_mm_castpd_pbf16(__m128d A) {
  // CHECK-LABEL: test_mm_castpd_pbf16
  // CHECK: bitcast <2 x double> %{{.*}} to <8 x bfloat>
  return _mm_castpd_pbf16(A);
}

__m256bf16 test_mm256_castpd_pbf16(__m256d A) {
  // CHECK-LABEL: test_mm256_castpd_pbf16
  // CHECK: bitcast <4 x double> %{{.*}} to <16 x bfloat>
  return _mm256_castpd_pbf16(A);
}

__m512bf16 test_mm512_castpd_pbf16(__m512d A) {
  // CHECK-LABEL: test_mm512_castpd_pbf16
  // CHECK: bitcast <8 x double> %{{.*}} to <32 x bfloat>
  return _mm512_castpd_pbf16(A);
}

__m128bf16 test_mm_castsi128_pbf16(__m128i A) {
  // CHECK-LABEL: test_mm_castsi128_pbf16
  // CHECK: bitcast <2 x i64> %{{.*}} to <8 x bfloat>
  return _mm_castsi128_pbf16(A);
}

__m256bf16 test_mm256_castsi256_pbf16(__m256i A) {
  // CHECK-LABEL: test_mm256_castsi256_pbf16
  // CHECK: bitcast <4 x i64> %{{.*}} to <16 x bfloat>
  return _mm256_castsi256_pbf16(A);
}

__m512bf16 test_mm512_castsi512_pbf16(__m512i A) {
  // CHECK-LABEL: test_mm512_castsi512_pbf16
  // CHECK: bitcast <8 x i64> %{{.*}} to <32 x bfloat>
  return _mm512_castsi512_pbf16(A);
}

__m128bf16 test_mm256_castpbf16256_pbf16128(__m256bf16 __a) {
  // CHECK-LABEL: test_mm256_castpbf16256_pbf16128
  // CHECK: shufflevector <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  return _mm256_castpbf16256_pbf16128(__a);
}

__m128bf16 test_mm512_castpbf16512_pbf16128(__m512bf16 __a) {
  // CHECK-LABEL: test_mm512_castpbf16512_pbf16128
  // CHECK: shufflevector <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  return _mm512_castpbf16512_pbf16128(__a);
}

__m256bf16 test_mm512_castpbf16512_pbf16256(__m512bf16 __a) {
  // CHECK-LABEL: test_mm512_castpbf16512_pbf16256
  // CHECK: shufflevector <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  return _mm512_castpbf16512_pbf16256(__a);
}

__m256bf16 test_mm256_castpbf16128_pbf16256(__m128bf16 __a) {
  // CHECK-LABEL: test_mm256_castpbf16128_pbf16256
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
  return _mm256_castpbf16128_pbf16256(__a);
}

__m512bf16 test_mm512_castpbf16128_pbf16512(__m128bf16 __a) {
  // CHECK-LABEL: test_mm512_castpbf16128_pbf16512
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
  return _mm512_castpbf16128_pbf16512(__a);
}

__m512bf16 test_mm512_castpbf16256_pbf16512(__m256bf16 __a) {
  // CHECK-LABEL: test_mm512_castpbf16256_pbf16512
  // CHECK: shufflevector <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
  return _mm512_castpbf16256_pbf16512(__a);
}

__m256bf16 test_mm256_zextpbf16128_pbf16256(__m128bf16 __a) {
  // CHECK-LABEL: test_mm256_zextpbf16128_pbf16256
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> {{.*}}, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  return _mm256_zextpbf16128_pbf16256(__a);
}

__m512bf16 test_mm512_zextpbf16128_pbf16512(__m128bf16 __a) {
  // CHECK-LABEL: test_mm512_zextpbf16128_pbf16512
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> {{.*}}, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  return _mm512_zextpbf16128_pbf16512(__a);
}

__m512bf16 test_mm512_zextpbf16256_pbf16512(__m256bf16 __a) {
  // CHECK-LABEL: test_mm512_zextpbf16256_pbf16512
  // CHECK: shufflevector <16 x bfloat> %{{.*}}, <16 x bfloat> {{.*}}, <32 x i32>
  return _mm512_zextpbf16256_pbf16512(__a);
}

__m512bf16 test_mm512_abs_pbf16(__m512bf16 a) {
  // CHECK-LABEL: @test_mm512_abs_pbf16
  // CHECK: and <16 x i32>
  return _mm512_abs_pbf16(a);
}

// VMOVSH

__m128bf16 test_mm_load_sbf16(void const *A) {
  // CHECK-LABEL: test_mm_load_sbf16
  // CHECK: %{{.*}} = call <8 x bfloat> @llvm.masked.load.v8bf16.p0(ptr %{{.*}}, i32 1, <8 x i1> bitcast (<1 x i8> <i8 1> to <8 x i1>), <8 x bfloat> %{{.*}})
  return _mm_load_sbf16(A);
}

__m128bf16 test_mm_mask_load_sbf16(__m128bf16 __A, __mmask8 __U, const void *__W) {
  // CHECK-LABEL: @test_mm_mask_load_sbf16
  // CHECK: %{{.*}} = call <8 x bfloat> @llvm.masked.load.v8bf16.p0(ptr %{{.*}}, i32 1, <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}})
  return _mm_mask_load_sbf16(__A, __U, __W);
}

__m128bf16 test_mm_maskz_load_sbf16(__mmask8 __U, const void *__W) {
  // CHECK-LABEL: @test_mm_maskz_load_sbf16
  // CHECK: %{{.*}} = call <8 x bfloat> @llvm.masked.load.v8bf16.p0(ptr %{{.*}}, i32 1, <8 x i1> %{{.*}}, <8 x bfloat> %{{.*}})
  return _mm_maskz_load_sbf16(__U, __W);
}

__m512bf16 test_mm512_load_pbf16(void *p) {
  // CHECK-LABEL: @test_mm512_load_pbf16
  // CHECK: load <32 x bfloat>, ptr %{{.*}}, align 64
  return _mm512_load_pbf16(p);
}

__m256bf16 test_mm256_load_pbf16(void *p) {
  // CHECK-LABEL: @test_mm256_load_pbf16
  // CHECK: load <16 x bfloat>, ptr %{{.*}}, align 32
  return _mm256_load_pbf16(p);
}

__m128bf16 test_mm_load_pbf16(void *p) {
  // CHECK-LABEL: @test_mm_load_pbf16
  // CHECK: load <8 x bfloat>, ptr %{{.*}}, align 16
  return _mm_load_pbf16(p);
}

__m512bf16 test_mm512_loadu_pbf16(void *p) {
  // CHECK-LABEL: @test_mm512_loadu_pbf16
  // CHECK: load <32 x bfloat>, ptr {{.*}}, align 1{{$}}
  return _mm512_loadu_pbf16(p);
}

__m256bf16 test_mm256_loadu_pbf16(void *p) {
  // CHECK-LABEL: @test_mm256_loadu_pbf16
  // CHECK: load <16 x bfloat>, ptr {{.*}}, align 1{{$}}
  return _mm256_loadu_pbf16(p);
}

__m128bf16 test_mm_loadu_pbf16(void *p) {
  // CHECK-LABEL: @test_mm_loadu_pbf16
  // CHECK: load <8 x bfloat>, ptr {{.*}}, align 1{{$}}
  return _mm_loadu_pbf16(p);
}

void test_mm_store_sbf16(void *A, __m128bf16 B) {
  // CHECK-LABEL: test_mm_store_sbf16
  // CHECK: extractelement <8 x bfloat> %{{.*}}, i32 0
  // CHECK: store bfloat %{{.*}}, ptr %{{.*}}, align 1{{$}}
  _mm_store_sbf16(A, B);
}

void test_mm_mask_store_sbf16(void *__P, __mmask8 __U, __m128bf16 __A) {
  // CHECK-LABEL: @test_mm_mask_store_sbf16
  // CHECK: call void @llvm.masked.store.v8bf16.p0(<8 x bfloat> %{{.*}}, ptr %{{.*}}, i32 1, <8 x i1> %{{.*}})
  _mm_mask_store_sbf16(__P, __U, __A);
}

void test_mm512_store_pbf16(void *p, __m512bf16 a) {
  // CHECK-LABEL: @test_mm512_store_pbf16
  // CHECK: store <32 x bfloat> %{{.*}}, ptr %{{.*}}, align 64
  _mm512_store_pbf16(p, a);
}

void test_mm256_store_pbf16(void *p, __m256bf16 a) {
  // CHECK-LABEL: @test_mm256_store_pbf16
  // CHECK: store <16 x bfloat> %{{.*}}, ptr %{{.*}}, align 32
  _mm256_store_pbf16(p, a);
}

void test_mm_store_pbf16(void *p, __m128bf16 a) {
  // CHECK-LABEL: @test_mm_store_pbf16
  // CHECK: store <8 x bfloat> %{{.*}}, ptr %{{.*}}, align 16
  _mm_store_pbf16(p, a);
}

void test_mm512_storeu_pbf16(void *p, __m512bf16 a) {
  // CHECK-LABEL: @test_mm512_storeu_pbf16
  // CHECK: store <32 x bfloat> %{{.*}}, ptr %{{.*}}, align 1{{$}}
  // CHECK-NEXT: ret void
  _mm512_storeu_pbf16(p, a);
}

void test_mm256_storeu_pbf16(void *p, __m256bf16 a) {
  // CHECK-LABEL: @test_mm256_storeu_pbf16
  // CHECK: store <16 x bfloat> %{{.*}}, ptr %{{.*}}, align 1{{$}}
  // CHECK-NEXT: ret void
  _mm256_storeu_pbf16(p, a);
}

void test_mm_storeu_pbf16(void *p, __m128bf16 a) {
  // CHECK-LABEL: @test_mm_storeu_pbf16
  // CHECK: store <8 x bfloat> %{{.*}}, ptr %{{.*}}, align 1{{$}}
  // CHECK-NEXT: ret void
  _mm_storeu_pbf16(p, a);
}

__m128bf16 test_mm_move_sbf16(__m128bf16 A, __m128bf16 B) {
  // CHECK-LABEL: test_mm_move_sbf16
  // CHECK: extractelement <8 x bfloat> %{{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat %{{.*}}, i32 0
  return _mm_move_sbf16(A, B);
}

__m128bf16 test_mm_mask_move_sbf16(__m128bf16 __W, __mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_mask_move_sbf16
  // CHECK: [[EXT:%.*]] = extractelement <8 x bfloat> %{{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <8 x bfloat> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <8 x bfloat> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %{{.*}}, bfloat [[A]], bfloat [[B]]
  // CHECK-NEXT: insertelement <8 x bfloat> [[VEC]], bfloat [[SEL]], i64 0
  return _mm_mask_move_sbf16(__W, __U, __A, __B);
}

__m128bf16 test_mm_maskz_move_sbf16(__mmask8 __U, __m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: @test_mm_maskz_move_sbf16
  // CHECK: [[EXT:%.*]] = extractelement <8 x bfloat> %{{.*}}, i32 0
  // CHECK: insertelement <8 x bfloat> %{{.*}}, bfloat [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <8 x bfloat> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <8 x bfloat> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %{{.*}}, bfloat [[A]], bfloat [[B]]
  // CHECK-NEXT: insertelement <8 x bfloat> [[VEC]], bfloat [[SEL]], i64 0
  return _mm_maskz_move_sbf16(__U, __A, __B);
}

__m512bf16 test_mm512_mask_blend_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __W) {
  // CHECK-LABEL: @test_mm512_mask_blend_pbf16
  // CHECK:  %{{.*}} = bitcast i32 %{{.*}} to <32 x i1>
  // CHECK:  %{{.*}} = select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_blend_pbf16(__U, __A, __W);
}

__m512bf16 test_mm512_permutex2var_pbf16(__m512bf16 __A, __m512i __I, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_permutex2var_pbf16
  // CHECK:  %{{.*}} = bitcast <32 x bfloat> %{{.*}} to <32 x i16>
  // CHECK:  %{{.*}} = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK:  %{{.*}} = bitcast <32 x bfloat> %{{.*}} to <32 x i16>
  // CHECK:  %{{.*}} = call <32 x i16> @llvm.x86.avx512.vpermi2var.hi.512(<32 x i16> %{{.*}}, <32 x i16> %{{.*}}, <32 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <32 x i16> %{{.*}} to <32 x bfloat>
  return _mm512_permutex2var_pbf16(__A, __I, __B);
}

__m512bf16 test_mm512_permutexvar_epi16(__m512i __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_permutexvar_epi16
  // CHECK:  %{{.*}} = bitcast <32 x bfloat> %{{.*}} to <32 x i16>
  // CHECK:  %{{.*}} = bitcast <8 x i64> %{{.*}} to <32 x i16>
  // CHECK:  %{{.*}} = call <32 x i16> @llvm.x86.avx512.permvar.hi.512(<32 x i16> %{{.*}}, <32 x i16> %{{.*}})
  // CHECK:  %{{.*}} = bitcast <32 x i16> %{{.*}} to <32 x bfloat>
  return _mm512_permutexvar_pbf16(__A, __B);
}

__m512bf16 test_mm512_addne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_addne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16512(
  return _mm512_addne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_addne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_addne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_addne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vaddnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_addne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_subne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_subne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16512(
  return _mm512_subne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_subne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_subne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_subne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vsubnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_subne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_mulne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_mulne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16512(
  return _mm512_mulne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_mulne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_mulne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_mulne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmulnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_mulne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_divne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_divne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16512(
  return _mm512_divne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_divne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_divne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_divne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vdivnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_divne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_maxne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_maxne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16512(
  return _mm512_maxne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_maxne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_maxne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_maxne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vmaxnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_maxne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_minne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_minne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16512(
  return _mm512_minne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_minne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_minne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_minne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK: @llvm.x86.avx512bf16ne.vminnepbf16512
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_minne_pbf16(__U, __A, __B);
}

int test_mm_comeqne_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comeqne_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16eq(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comeqne_sbf16(__A, __B);
}

int test_mm_comltne_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comltne_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16lt(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comltne_sbf16(__A, __B);
}

int test_mm_comlene_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comlene_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16le(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comlene_sbf16(__A, __B);
}

int test_mm_comgtne_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comgtne_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16gt(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comgtne_sbf16(__A, __B);
}

int test_mm_comgene_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comgene_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16ge(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comgene_sbf16(__A, __B);
}

int test_mm_comneqne_sbf16(__m128bf16 __A, __m128bf16 __B) {
  // CHECK-LABEL: test_mm_comneqne_sbf16
  // CHECK: %{{.}} = call i32 @llvm.x86.avx512bf16ne.vcomnesbf16neq(<8 x bfloat> %{{.}}, <8 x bfloat> %{{.}})
  return _mm_comneqne_sbf16(__A, __B);
}

__mmask32 test_mm512_cmpne_pbf16_mask_eq_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: @test_mm512_cmpne_pbf16_mask_eq_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 0, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_EQ_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_lt_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_lt_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 1, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_LT_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_le_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_le_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 2, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_LE_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_unord_q(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_unord_q
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 3, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_UNORD_Q);
}

__mmask32 test_mm512_cmpne_pbf16_mask_neq_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_neq_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 4, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NEQ_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nlt_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nlt_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 5, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NLT_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nle_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nle_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 6, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NLE_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ord_q(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ord_q
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 7, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_ORD_Q);
}

__mmask32 test_mm512_cmpne_pbf16_mask_eq_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_eq_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 8, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_EQ_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nge_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nge_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 9, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NGE_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ngt_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ngt_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 10, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NGT_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_false_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_false_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 11, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_FALSE_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_neq_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_neq_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 12, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NEQ_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ge_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ge_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 13, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_GE_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_gt_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_gt_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 14, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_GT_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_true_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_true_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 15, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_TRUE_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_eq_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_eq_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 16, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_EQ_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_lt_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_lt_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 17, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_LT_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_le_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_le_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 18, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_LE_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_unord_s(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_unord_s
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 19, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_UNORD_S);
}

__mmask32 test_mm512_cmpne_pbf16_mask_neq_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_neq_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 20, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NEQ_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nlt_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 21, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NLT_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nle_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nle_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 22, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NLE_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ord_s(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ord_s
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 23, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_ORD_S);
}

__mmask32 test_mm512_cmpne_pbf16_mask_eq_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_eq_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 24, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_EQ_US);
}

__mmask32 test_mm512_cmpne_pbf16_mask_nge_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_nge_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 25, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NGE_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ngt_uq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 26, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NGT_UQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_false_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_false_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 27, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_FALSE_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_neq_os(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_neq_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 28, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_NEQ_OS);
}

__mmask32 test_mm512_cmpne_pbf16_mask_ge_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_ge_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 29, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_GE_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_gt_oq(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_gt_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 30, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_GT_OQ);
}

__mmask32 test_mm512_cmpne_pbf16_mask_true_us(__m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_cmpne_pbf16_mask_true_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 31, i32 -1)
  return _mm512_cmpne_pbf16_mask(a, b, _CMP_TRUE_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_eq_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: @test_mm512_mask_cmpne_pbf16_mask_eq_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 0, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_lt_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_lt_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 1, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_le_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_le_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 2, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_unord_q(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_unord_q
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 3, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_Q);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_neq_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_neq_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 4, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nlt_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nlt_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 5, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nle_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nle_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 6, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ord_q(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ord_q
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 7, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_Q);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_eq_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_eq_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 8, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nge_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nge_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 9, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ngt_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ngt_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 10, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_false_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_false_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 11, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_neq_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_neq_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 12, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ge_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ge_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 13, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_gt_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_gt_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 14, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_true_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_true_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 15, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_eq_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_eq_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 16, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_lt_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_lt_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 17, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_LT_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_le_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_le_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 18, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_LE_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_unord_s(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_unord_s
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 19, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_UNORD_S);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_neq_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_neq_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 20, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nlt_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nlt_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 21, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLT_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nle_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nle_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 22, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NLE_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ord_s(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ord_s
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 23, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_ORD_S);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_eq_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_eq_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 24, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_EQ_US);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_nge_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_nge_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 25, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGE_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ngt_uq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ngt_uq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 26, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NGT_UQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_false_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_false_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 27, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_FALSE_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_neq_os(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_neq_os
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 28, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_NEQ_OS);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_ge_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_ge_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 29, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_GE_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_gt_oq(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_gt_oq
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 30, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_GT_OQ);
}

__mmask32 test_mm512_mask_cmpne_pbf16_mask_true_us(__mmask32 m, __m512bf16 a, __m512bf16 b) {
  // CHECK-LABEL: test_mm512_mask_cmpne_pbf16_mask_true_us
  // CHECK: call i32 @llvm.x86.avx512bf16ne.vcmpnepbf16512.mask(<32 x bfloat> %{{.}}, <32 x bfloat> %{{.}}, i32 31, i32 %{{.}})
  return _mm512_mask_cmpne_pbf16_mask(m, a, b, _CMP_TRUE_US);
}

__mmask32 test_mm512_mask_fpclassne_pbf16_mask(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.512
  return _mm512_mask_fpclassne_pbf16_mask(__U, __A, 4);
}

__mmask32 test_mm512_fpclassne_pbf16_mask(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_fpclassne_pbf16_mask
  // CHECK: @llvm.x86.avx512bf16ne.fpclass.nepbf16.512
  return _mm512_fpclassne_pbf16_mask(__A, 4);
}

__m512bf16 test_mm512_scalefne_pbf16(__m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.512
  return _mm512_scalefne_pbf16(__A, __B);
}

__m512bf16 test_mm512_mask_scalefne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_mask_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.512
  return _mm512_mask_scalefne_pbf16(__W, __U, __A, __B);
}

__m512bf16 test_mm512_maskz_scalefne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B) {
  // CHECK-LABEL: @test_mm512_maskz_scalefne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.scalef.nepbf16.512
  return _mm512_maskz_scalefne_pbf16(__U, __A, __B);
}

__m512bf16 test_mm512_rcpne_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.512
  return _mm512_rcpne_pbf16(__A);
}

__m512bf16 test_mm512_mask_rcpne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.512
  return (__m512bf16)_mm512_mask_rcpne_pbf16(__W, __U, __A);
}

__m512bf16 test_mm512_maskz_rcpne_pbf16(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_rcpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rcp.nepbf16.512
  return _mm512_maskz_rcpne_pbf16(__U, __A);
}

__m512bf16 test_mm512_getexpne_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.512
  return _mm512_getexpne_pbf16(__A);
}

__m512bf16 test_mm512_mask_getexpne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.512
  return _mm512_mask_getexpne_pbf16(__W, __U, __A);
}

__m512bf16 test_mm512_maskz_getexpne_pbf16(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_getexpne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getexp.nepbf16.512
  return _mm512_maskz_getexpne_pbf16(__U, __A);
}

__m512bf16 test_mm512_rsqrtne_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.512
  return _mm512_rsqrtne_pbf16(__A);
}

__m512bf16 test_mm512_mask_rsqrtne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.512
  return (__m512bf16)_mm512_mask_rsqrtne_pbf16(__W, __U, __A);
}

__m512bf16 test_mm512_maskz_rsqrtne_pbf16(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_rsqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rsqrt.nepbf16.512
  return _mm512_maskz_rsqrtne_pbf16(__U, __A);
}

__m512bf16 test_mm512_reducene_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.512
  return _mm512_reducene_pbf16(__A, 3);
}

__m512bf16 test_mm512_mask_reducene_pbf16(__m512bf16 __W, __mmask16 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.512
  return _mm512_mask_reducene_pbf16(__W, __U, __A, 1);
}

__m512bf16 test_mm512_maskz_reducene_pbf16(__mmask16 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_reducene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.reduce.nepbf16.512
  return _mm512_maskz_reducene_pbf16(__U, __A, 1);
}

__m512bf16 test_mm512_roundscalene_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.512
  return _mm512_roundscalene_pbf16(__A, 3);
}

__m512bf16 test_mm512_mask_roundscalene_pbf16(__m512bf16 __W, __mmask16 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.512
  return _mm512_mask_roundscalene_pbf16(__W, __U, __A, 1);
}

__m512bf16 test_mm512_maskz_roundscalene_pbf16(__mmask16 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_roundscalene_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.rndscale.nepbf16.512
  return _mm512_maskz_roundscalene_pbf16(__U, __A, 1 );
}

__m512bf16 test_mm512_getmantne_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.512
  return _mm512_getmantne_pbf16(__A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m512bf16 test_mm512_mask_getmantne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.512
  return _mm512_mask_getmantne_pbf16(__W, __U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m512bf16 test_mm512_maskz_getmantne_pbf16(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_getmantne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.getmant.nepbf16.512
  return _mm512_maskz_getmantne_pbf16(__U, __A, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m512bf16 test_mm512_sqrtne_pbf16(__m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.512
  return _mm512_sqrtne_pbf16(__A);
}

__m512bf16 test_mm512_mask_sqrtne_pbf16(__m512bf16 __W, __mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_mask_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.512
  return (__m512bf16)_mm512_mask_sqrtne_pbf16(__W, __U, __A);
}

__m512bf16 test_mm512_maskz_sqrtne_pbf16(__mmask32 __U, __m512bf16 __A) {
  // CHECK-LABEL: @test_mm512_maskz_sqrtne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.mask.sqrt.nepbf16.512
  return _mm512_maskz_sqrtne_pbf16(__U, __A);
}

__m512bf16 test_mm512_fmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16512(
  return _mm512_fmaddne_pbf16(__A, __B, __C);
}

__m512bf16 test_mm512_mask_fmaddne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_mask_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd132nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_fmaddne_pbf16(__A, __U, __B, __C);
}

__m512bf16 test_mm512_mask3_fmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C, __mmask32 __U) {
  // CHECK-LABEL: @test_mm512_mask3_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd231nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask3_fmaddne_pbf16(__A, __B, __C, __U);
}

__m512bf16 test_mm512_maskz_fmaddne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_maskz_fmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmadd213nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_fmaddne_pbf16(__U, __A, __B, __C);
}

__m512bf16 test_mm512_fmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16512(
  return _mm512_fmsubne_pbf16(__A, __B, __C);
}

__m512bf16 test_mm512_mask_fmsubne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_mask_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub132nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_fmsubne_pbf16(__A, __U, __B, __C);
}

__m512bf16 test_mm512_mask3_fmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C, __mmask32 __U) {
  // CHECK-LABEL: @test_mm512_mask3_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub231nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask3_fmsubne_pbf16(__A, __B, __C, __U);
}

__m512bf16 test_mm512_maskz_fmsubne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_maskz_fmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfmsub213nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_fmsubne_pbf16(__U, __A, __B, __C);
}

__m512bf16 test_mm512_fnmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16512(
  return _mm512_fnmaddne_pbf16(__A, __B, __C);
}

__m512bf16 test_mm512_mask_fnmaddne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_mask_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd132nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_fnmaddne_pbf16(__A, __U, __B, __C);
}

__m512bf16 test_mm512_mask3_fnmaddne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C, __mmask32 __U) {
  // CHECK-LABEL: @test_mm512_mask3_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd231nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask3_fnmaddne_pbf16(__A, __B, __C, __U);
}

__m512bf16 test_mm512_maskz_fnmaddne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_maskz_fnmaddne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmadd213nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_fnmaddne_pbf16(__U, __A, __B, __C);
}

__m512bf16 test_mm512_fnmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16512(
  return _mm512_fnmsubne_pbf16(__A, __B, __C);
}

__m512bf16 test_mm512_mask_fnmsubne_pbf16(__m512bf16 __A, __mmask32 __U, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_mask_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub132nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask_fnmsubne_pbf16(__A, __U, __B, __C);
}

__m512bf16 test_mm512_mask3_fnmsubne_pbf16(__m512bf16 __A, __m512bf16 __B, __m512bf16 __C, __mmask32 __U) {
  // CHECK-LABEL: @test_mm512_mask3_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub231nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_mask3_fnmsubne_pbf16(__A, __B, __C, __U);
}

__m512bf16 test_mm512_maskz_fnmsubne_pbf16(__mmask32 __U, __m512bf16 __A, __m512bf16 __B, __m512bf16 __C) {
  // CHECK-LABEL: @test_mm512_maskz_fnmsubne_pbf16
  // CHECK: @llvm.x86.avx512bf16ne.vfnmsub213nepbf16512(
  // CHECK: select <32 x i1> %{{.*}}, <32 x bfloat> %{{.*}}, <32 x bfloat> %{{.*}}
  return _mm512_maskz_fnmsubne_pbf16(__U, __A, __B, __C);
}
