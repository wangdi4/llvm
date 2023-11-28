// REQUIRES: intel_feature_isa_avx256p
// RUN: %clang_cc1 -ffreestanding -flax-vector-conversions=none %s -triple=x86_64-unknown-unknown -target-feature +avx256p -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

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

__m128bf16 test_mm256_castpbf16256_pbf16128(__m256bf16 __a) {
  // CHECK-LABEL: test_mm256_castpbf16256_pbf16128
  // CHECK: shufflevector <16 x bfloat> %{{.*}}, <16 x bfloat> %{{.*}}, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  return _mm256_castpbf16256_pbf16128(__a);
}

__m256bf16 test_mm256_castpbf16128_pbf16256(__m128bf16 __a) {
  // CHECK-LABEL: test_mm256_castpbf16128_pbf16256
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> %{{.*}}, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison, i32 poison>
  return _mm256_castpbf16128_pbf16256(__a);
}

__m256bf16 test_mm256_zextpbf16128_pbf16256(__m128bf16 __a) {
  // CHECK-LABEL: test_mm256_zextpbf16128_pbf16256
  // CHECK: shufflevector <8 x bfloat> %{{.*}}, <8 x bfloat> {{.*}}, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
  return _mm256_zextpbf16128_pbf16256(__a);
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