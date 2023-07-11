// REQUIRES: intel_feature_isa_avx_compress
// RUN: %clang_cc1 -no-opaque-pointers -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avxcompress -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>

__m128i test_mm_maskload_epi8(char const *a, __m128i m) {
  // CHECK-LABEL: test_mm_maskload_epi8
  // CHECK: call <16 x i8> @llvm.x86.avx2.maskload.b(i8* %{{.*}}, <16 x i8> %{{.*}})
  return _mm_maskload_epi8(a, m);
}

__m256i test_mm256_maskload_epi8(char const *a, __m256i m) {
  // CHECK-LABEL: test_mm256_maskload_epi8
  // CHECK: call <32 x i8> @llvm.x86.avx2.maskload.b.256(i8* %{{.*}}, <32 x i8> %{{.*}})
  return _mm256_maskload_epi8(a, m);
}

__m128i test_mm_maskload_epi16(short const *a, __m128i m) {
  // CHECK-LABEL: test_mm_maskload_epi16
  // CHECK: call <8 x i16> @llvm.x86.avx2.maskload.w(i8* %{{.*}}, <8 x i16> %{{.*}})
  return _mm_maskload_epi16(a, m);
}

__m256i test_mm256_maskload_epi16(short const *a, __m256i m) {
  // CHECK-LABEL: test_mm256_maskload_epi16
  // CHECK: call <16 x i16> @llvm.x86.avx2.maskload.w.256(i8* %{{.*}}, <16 x i16> %{{.*}})
  return _mm256_maskload_epi16(a, m);
}

void test_mm_maskstore_epi8(char *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_maskstore_epi8
  // CHECK: call void @llvm.x86.avx2.maskstore.b(i8* %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  _mm_maskstore_epi8(a, m, b);
}

void test_mm256_maskstore_epi8(char *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_maskstore_epi8
  // CHECK: call void @llvm.x86.avx2.maskstore.b.256(i8* %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}})
  _mm256_maskstore_epi8(a, m, b);
}

void test_mm_maskstore_epi16(short *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_maskstore_epi16
  // CHECK: call void @llvm.x86.avx2.maskstore.w(i8* %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  _mm_maskstore_epi16(a, m, b);
}

void test_mm256_maskstore_epi16(short *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_maskstore_epi16
  // CHECK: call void @llvm.x86.avx2.maskstore.w.256(i8* %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  _mm256_maskstore_epi16(a, m, b);
}

void test_mm_compress_store_epi8(char *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_compress_store_epi8
  // CHECK: call void @llvm.x86.avx2.vpcompressb.store.128(i8* %{{.*}}, <16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  _mm_compress_store_epi8(a, m, b);
}

void test_mm_compress_store_epi16(short *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_compress_store_epi16
  // CHECK: call void @llvm.x86.avx2.vpcompressw.store.128(i8* %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  _mm_compress_store_epi16(a, m, b);
}

void test_mm_compress_store_epi32(int *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_compress_store_epi32
  // CHECK: call void @llvm.x86.avx2.vpcompressd.store.128(i8* %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  _mm_compress_store_epi32(a, m, b);
}

void test_mm_compress_store_epi64(long long *a, __m128i m, __m128i b) {
  // CHECK-LABEL: test_mm_compress_store_epi64
  // CHECK: call void @llvm.x86.avx2.vpcompressq.store.128(i8* %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  _mm_compress_store_epi64(a, m, b);
}

void test_mm256_compress_store_epi8(char *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_compress_store_epi8
  // CHECK: call void @llvm.x86.avx2.vpcompressb.store.256(i8* %{{.*}}, <32 x i8> %{{.*}}, <32 x i8> %{{.*}})
  _mm256_compress_store_epi8(a, m, b);
}

void test_mm256_compress_store_epi16(short *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_compress_store_epi16
  // CHECK: call void @llvm.x86.avx2.vpcompressw.store.256(i8* %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  _mm256_compress_store_epi16(a, m, b);
}

void test_mm256_compress_store_epi32(int *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_compress_store_epi32
  // CHECK: call void @llvm.x86.avx2.vpcompressd.store.256(i8* %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  _mm256_compress_store_epi32(a, m, b);
}

void test_mm256_compress_store_epi64(long long *a, __m256i m, __m256i b) {
  // CHECK-LABEL: test_mm256_compress_store_epi64
  // CHECK: call void @llvm.x86.avx2.vpcompressq.store.256(i8* %{{.*}}, <4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  _mm256_compress_store_epi64(a, m, b);
}

__m128i test_mm_compress_epi8(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_compress_epi8
  // CHECK: call <16 x i8> @llvm.x86.avx2.vpcompressb.128(<16 x i8> %{{.*}}, <16 x i8> %{{.*}})
  return _mm_compress_epi8(x, y);
}

__m128i test_mm_compress_epi16(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_compress_epi16
  // CHECK: call <8 x i16> @llvm.x86.avx2.vpcompressw.128(<8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_compress_epi16(x, y);
}

__m128i test_mm_compress_epi32(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_compress_epi32
  // CHECK: call <4 x i32> @llvm.x86.avx2.vpcompressd.128(<4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_compress_epi32(x, y);
}

__m128i test_mm_compress_epi64(__m128i x, __m128i y) {
  // CHECK-LABEL: test_mm_compress_epi64
  // CHECK: call <2 x i64> @llvm.x86.avx2.vpcompressq.128(<2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_compress_epi64(x, y);
}

__m256i test_mm256_compress_epi8(__m256i x, __m256i y) {
  // CHECK-LABEL: test_mm256_compress_epi8
  // CHECK: call <32 x i8> @llvm.x86.avx2.vpcompressb.256(<32 x i8> %{{.*}}, <32 x i8> %{{.*}})
  return _mm256_compress_epi8(x, y);
}

__m256i test_mm256_compress_epi16(__m256i x, __m256i y) {
  // CHECK-LABEL: test_mm256_compress_epi16
  // CHECK: call <16 x i16> @llvm.x86.avx2.vpcompressw.256(<16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  return _mm256_compress_epi16(x, y);
}

__m256i test_mm256_compress_epi32(__m256i x, __m256i y) {
  // CHECK-LABEL: test_mm256_compress_epi32
  // CHECK: call <8 x i32> @llvm.x86.avx2.vpcompressd.256(<8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_compress_epi32(x, y);
}

__m256i test_mm256_compress_epi64(__m256i x, __m256i y) {
  // CHECK-LABEL: test_mm256_compress_epi64
  // CHECK: call <4 x i64> @llvm.x86.avx2.vpcompressq.256(<4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_compress_epi64(x, y);
}

__m128i test_mm_popcnt_avx_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_popcnt_avx_epi64
  // CHECK: @llvm.ctpop.v2i64
  return _mm_popcnt_avx_epi64(__A);
}

__m128i test_mm_popcnt_avx_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_popcnt_avx_epi32
  // CHECK: @llvm.ctpop.v4i32
  return _mm_popcnt_avx_epi32(__A);
}

__m256i test_mm256_popcnt_avx_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_popcnt_avx_epi64
  // CHECK: @llvm.ctpop.v4i64
  return _mm256_popcnt_avx_epi64(__A);
}

__m256i test_mm256_popcnt_avx_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_popcnt_avx_epi32
  // CHECK: @llvm.ctpop.v8i32
  return _mm256_popcnt_avx_epi32(__A);
}

__m128i test_mm_lzcnt_avx_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_lzcnt_avx_epi32
  // CHECK: call <4 x i32> @llvm.ctlz.v4i32(<4 x i32> %{{.*}}, i1 false)
  return _mm_lzcnt_avx_epi32(__A);
}

__m256i test_mm256_lzcnt_avx_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_lzcnt_avx_epi32
  // CHECK: call <8 x i32> @llvm.ctlz.v8i32(<8 x i32> %{{.*}}, i1 false)
  return _mm256_lzcnt_avx_epi32(__A);
}

__m128i test_mm_lzcnt_avx_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_lzcnt_avx_epi64
  // CHECK: call <2 x i64> @llvm.ctlz.v2i64(<2 x i64> %{{.*}}, i1 false)
  return _mm_lzcnt_avx_epi64(__A);
}

__m256i test_mm256_lzcnt_avx_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_lzcnt_avx_epi64
  // CHECK: call <4 x i64> @llvm.ctlz.v4i64(<4 x i64> %{{.*}}, i1 false)
  return _mm256_lzcnt_avx_epi64(__A);
}

__m256i test_mm256_popcnt_avx_epi16(__m256i __A) {
  // CHECK-LABEL: @test_mm256_popcnt_avx_epi16
  // CHECK: @llvm.ctpop.v16i16
  return _mm256_popcnt_avx_epi16(__A);
}

__m128i test_mm_popcnt_avx_epi16(__m128i __A) {
  // CHECK-LABEL: @test_mm_popcnt_avx_epi16
  // CHECK: @llvm.ctpop.v8i16
  return _mm_popcnt_avx_epi16(__A);
}

__m256i test_mm256_popcnt_avx_epi8(__m256i __A) {
  // CHECK-LABEL: @test_mm256_popcnt_avx_epi8
  // CHECK: @llvm.ctpop.v32i8
  return _mm256_popcnt_avx_epi8(__A);
}

__m128i test_mm_popcnt_avx_epi8(__m128i __A) {
  // CHECK-LABEL: @test_mm_popcnt_avx_epi8
  // CHECK: @llvm.ctpop.v16i8
  return _mm_popcnt_avx_epi8(__A);
}

__m256i test_mm256_shldi_avx_epi64(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldi_avx_epi64
  // CHECK: @llvm.fshl.v4i64(<4 x i64> %{{.*}}, <4 x i64> %{{.*}}, <4 x i64> <i64 31, i64 31, i64 31, i64 31>)
  return _mm256_shldi_avx_epi64(__A, __B, 31);
}

__m128i test_mm_shldi_avx_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldi_avx_epi64
  // CHECK: @llvm.fshl.v2i64(<2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> <i64 31, i64 31>)
  return _mm_shldi_avx_epi64(__A, __B, 31);
}

__m256i test_mm256_shldi_avx_epi32(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldi_avx_epi32
  // CHECK: @llvm.fshl.v8i32(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>)
  return _mm256_shldi_avx_epi32(__A, __B, 31);
}

__m128i test_mm_shldi_avx_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldi_avx_epi32
  // CHECK: @llvm.fshl.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> <i32 31, i32 31, i32 31, i32 31>)
  return _mm_shldi_avx_epi32(__A, __B, 31);
}

__m256i test_mm256_shldi_avx_epi16(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldi_avx_epi16
  // CHECK: @llvm.fshl.v16i16(<16 x i16> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>)
  return _mm256_shldi_avx_epi16(__A, __B, 31);
}

__m128i test_mm_shldi_avx_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldi_avx_epi16
  // CHECK: @llvm.fshl.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>)
  return _mm_shldi_avx_epi16(__A, __B, 31);
}

__m256i test_mm256_shrdi_avx_epi64(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdi_avx_epi64
  // CHECK: @llvm.fshr.v4i64(<4 x i64> %{{.*}}, <4 x i64> %{{.*}}, <4 x i64> <i64 31, i64 31, i64 31, i64 31>)
  return _mm256_shrdi_avx_epi64(__A, __B, 31);
}

__m128i test_mm_shrdi_avx_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdi_avx_epi64
  // CHECK: @llvm.fshr.v2i64(<2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> <i64 31, i64 31>)
  return _mm_shrdi_avx_epi64(__A, __B, 31);
}

__m256i test_mm256_shrdi_avx_epi32(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdi_avx_epi32
  // CHECK: @llvm.fshr.v8i32(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> <i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31, i32 31>)
  return _mm256_shrdi_avx_epi32(__A, __B, 31);
}

__m128i test_mm_shrdi_avx_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdi_avx_epi32
  // CHECK: @llvm.fshr.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> <i32 31, i32 31, i32 31, i32 31>)
  return _mm_shrdi_avx_epi32(__A, __B, 31);
}

__m256i test_mm256_shrdi_avx_epi16(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdi_avx_epi16
  // CHECK: @llvm.fshr.v16i16(<16 x i16> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>
  return _mm256_shrdi_avx_epi16(__A, __B, 31);
}

__m128i test_mm_shrdi_avx_epi16(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdi_avx_epi16
  // CHECK: @llvm.fshr.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> <i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31, i16 31>)
  return _mm_shrdi_avx_epi16(__A, __B, 31);
}


__m256i test_mm256_shldv_avx_epi64(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldv_avx_epi64
  // CHECK: @llvm.fshl.v4i64(<4 x i64> %{{.*}}, <4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_shldv_avx_epi64(__S, __A, __B);
}

__m128i test_mm_shldv_avx_epi64(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldv_avx_epi64
  // CHECK: @llvm.fshl.v2i64(<2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_shldv_avx_epi64(__S, __A, __B);
}

__m256i test_mm256_shldv_avx_epi32(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldv_avx_epi32
  // CHECK: @llvm.fshl.v8i32(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_shldv_avx_epi32(__S, __A, __B);
}

__m128i test_mm_shldv_avx_epi32(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldv_avx_epi32
  // CHECK: @llvm.fshl.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_shldv_avx_epi32(__S, __A, __B);
}

__m256i test_mm256_shldv_avx_epi16(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shldv_avx_epi16
  // CHECK: @llvm.fshl.v16i16(<16 x i16> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  return _mm256_shldv_avx_epi16(__S, __A, __B);
}

__m128i test_mm_shldv_avx_epi16(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shldv_avx_epi16
  // CHECK: @llvm.fshl.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_shldv_avx_epi16(__S, __A, __B);
}

__m256i test_mm256_shrdv_avx_epi64(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdv_avx_epi64
  // CHECK: @llvm.fshr.v4i64(<4 x i64> %{{.*}}, <4 x i64> %{{.*}}, <4 x i64> %{{.*}})
  return _mm256_shrdv_avx_epi64(__S, __A, __B);
}

__m128i test_mm_shrdv_avx_epi64(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdv_avx_epi64
  // CHECK: @llvm.fshr.v2i64(<2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_shrdv_avx_epi64(__S, __A, __B);
}

__m256i test_mm256_shrdv_avx_epi32(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdv_avx_epi32
  // CHECK: @llvm.fshr.v8i32(<8 x i32> %{{.*}}, <8 x i32> %{{.*}}, <8 x i32> %{{.*}})
  return _mm256_shrdv_avx_epi32(__S, __A, __B);
}

__m128i test_mm_shrdv_avx_epi32(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdv_avx_epi32
  // CHECK: @llvm.fshr.v4i32(<4 x i32> %{{.*}}, <4 x i32> %{{.*}}, <4 x i32> %{{.*}})
  return _mm_shrdv_avx_epi32(__S, __A, __B);
}

__m256i test_mm256_shrdv_avx_epi16(__m256i __S, __m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_shrdv_avx_epi16
  // CHECK: @llvm.fshr.v16i16(<16 x i16> %{{.*}}, <16 x i16> %{{.*}}, <16 x i16> %{{.*}})
  return _mm256_shrdv_avx_epi16(__S, __A, __B);
}

__m128i test_mm_shrdv_avx_epi16(__m128i __S, __m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_shrdv_avx_epi16
  // CHECK: @llvm.fshr.v8i16(<8 x i16> %{{.*}}, <8 x i16> %{{.*}}, <8 x i16> %{{.*}})
  return _mm_shrdv_avx_epi16(__S, __A, __B);
}

__m128i test_mm_rol_avx_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_rol_avx_epi32
  // CHECK: @llvm.fshl.v4i32
  return _mm_rol_avx_epi32(__A, 5);
}

__m256i test_mm256_rol_avx_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_rol_avx_epi32
  // CHECK: @llvm.fshl.v8i32
  return _mm256_rol_avx_epi32(__A, 5);
}

__m128i test_mm_rol_avx_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_rol_avx_epi64
  // CHECK: @llvm.fshl.v2i64
  return _mm_rol_avx_epi64(__A, 5);
}

__m256i test_mm256_rol_avx_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_rol_avx_epi64
  // CHECK: @llvm.fshl.v4i64
  return _mm256_rol_avx_epi64(__A, 5);
}

__m128i test_mm_rolv_avx_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_rolv_avx_epi32
  // CHECK: llvm.fshl.v4i32
  return _mm_rolv_avx_epi32(__A, __B);
}

__m256i test_mm256_rolv_avx_epi32(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_rolv_avx_epi32
  // CHECK: @llvm.fshl.v8i32
  return _mm256_rolv_avx_epi32(__A, __B);
}

__m128i test_mm_rolv_avx_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_rolv_avx_epi64
  // CHECK: @llvm.fshl.v2i64
  return _mm_rolv_avx_epi64(__A, __B);
}

__m256i test_mm256_rolv_avx_epi64(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_rolv_avx_epi64
  // CHECK: @llvm.fshl.v4i64
  return _mm256_rolv_avx_epi64(__A, __B);
}

__m128i test_mm_ror_avx_epi32(__m128i __A) {
  // CHECK-LABEL: @test_mm_ror_avx_epi32
  // CHECK: @llvm.fshr.v4i32
  return _mm_ror_avx_epi32(__A, 5);
}

__m256i test_mm256_ror_avx_epi32(__m256i __A) {
  // CHECK-LABEL: @test_mm256_ror_avx_epi32
  // CHECK: @llvm.fshr.v8i32
  return _mm256_ror_avx_epi32(__A, 5);
}

__m128i test_mm_ror_avx_epi64(__m128i __A) {
  // CHECK-LABEL: @test_mm_ror_avx_epi64
  // CHECK: @llvm.fshr.v2i64
  return _mm_ror_avx_epi64(__A, 5);
}

__m256i test_mm256_ror_avx_epi64(__m256i __A) {
  // CHECK-LABEL: @test_mm256_ror_avx_epi64
  // CHECK: @llvm.fshr.v4i64
  return _mm256_ror_avx_epi64(__A, 5);
}

__m128i test_mm_rorv_avx_epi32(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_rorv_avx_epi32
  // CHECK: @llvm.fshr.v4i32
  return _mm_rorv_avx_epi32(__A, __B);
}

__m256i test_mm256_rorv_avx_epi32(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_rorv_avx_epi32
  // CHECK: @llvm.fshr.v8i32
  return _mm256_rorv_avx_epi32(__A, __B);
}

__m128i test_mm_rorv_avx_epi64(__m128i __A, __m128i __B) {
  // CHECK-LABEL: @test_mm_rorv_avx_epi64
  // CHECK: @llvm.fshr.v2i64
  return _mm_rorv_avx_epi64(__A, __B);
}

__m256i test_mm256_rorv_avx_epi64(__m256i __A, __m256i __B) {
  // CHECK-LABEL: @test_mm256_rorv_avx_epi64
  // CHECK: @llvm.fshr.v4i64
  return _mm256_rorv_avx_epi64(__A, __B);
}

