// REQUIRES: intel_feature_isa_avx_memadvise
// RUN: %clang_cc1 -no-opaque-pointers -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avxmemadvise -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>
__m128i test_mm_vmovadvisew_load_epi8(__m128i const *a) {
  // CHECK-LABEL: test_mm_vmovadvisew_load_epi8
  // CHECK: call <4 x i32> @llvm.x86.avx2.vmovadvisew.load.128(i8* %{{.*}}, i8 16)
  return _mm_vmovadvisew_load_epi8(a, 16);
}

void test_mm_vmovadvisew_store_epi8(__m128i *a, __m128i m) {
  // CHECK-LABEL: test_mm_vmovadvisew_store_epi8
  // CHECK: call void @llvm.x86.avx2.vmovadvisew.store.128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 16)
  _mm_vmovadvisew_store_epi8(a, m, 16);
}

__m256i test_mm256_vmovadvisew_load_epi8(__m256i const *a) {
  // CHECK-LABEL: test_mm256_vmovadvisew_load_epi8
  // CHECK: call <8 x i32> @llvm.x86.avx2.vmovadvisew.load.256(i8* %{{.*}}, i8 16)
  return _mm256_vmovadvisew_load_epi8(a, 16);
}

void test_mm256_vmovadvisew_store_epi8(__m256i *a, __m256i m) {
  // CHECK-LABEL: test_mm256_vmovadvisew_store_epi8
  // CHECK: call void @llvm.x86.avx2.vmovadvisew.store.256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 16)
  _mm256_vmovadvisew_store_epi8(a, m, 16);
}

void test_mm_vmemadvise_epi8(__m128i *a) {
  // CHECK-LABEL: test_mm_vmemadvise_epi8
  // CHECK: call void @llvm.x86.avx2.vmemadvise.128(i8* %{{.*}}, i8 16)
  _mm_vmemadvise_epi8(a, 16);
}

void test_mm256_vmemadvise_epi8(__m256i *a) {
  // CHECK-LABEL: test_mm256_vmemadvise_epi8
  // CHECK: call void @llvm.x86.avx2.vmemadvise.256(i8* %{{.*}}, i8 16)
  _mm256_vmemadvise_epi8(a, 16);
}

__m128i test_mm_vmovadvisew_load_avx_epi8(__m128i const *a) {
  // CHECK-LABEL: test_mm_vmovadvisew_load_avx_epi8
  // CHECK: call <4 x i32> @llvm.x86.avx2.vmovadvisew.load.128(i8* %{{.*}}, i8 16)
  return _mm_vmovadvisew_load_avx_epi8(a, 16);
}

void test_mm_vmovadvisew_store_avx_epi8(__m128i *a, __m128i m) {
  // CHECK-LABEL: test_mm_vmovadvisew_store_avx_epi8
  // CHECK: call void @llvm.x86.avx2.vmovadvisew.store.128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 16)
  _mm_vmovadvisew_store_avx_epi8(a, m, 16);
}

void test_mm_vmemadvise_avx_epi8(const __m128i *a) {
  // CHECK-LABEL: test_mm_vmemadvise_avx_epi8
  // CHECK: call void @llvm.x86.avx2.vmemadvise.128(i8* %{{.*}}, i8 16)
  _mm_vmemadvise_avx_epi8(a, 16);
}

__m256i test_mm256_vmovadvisew_load_avx_epi8(__m256i const *a) {
  // CHECK-LABEL: test_mm256_vmovadvisew_load_avx_epi8
  // CHECK: call <8 x i32> @llvm.x86.avx2.vmovadvisew.load.256(i8* %{{.*}}, i8 16)
  return _mm256_vmovadvisew_load_avx_epi8(a, 16);
}

void test_mm256_vmovadvisew_store_avx_epi8(__m256i *a, __m256i m) {
  // CHECK-LABEL: test_mm256_vmovadvisew_store_avx_epi8
  // CHECK: call void @llvm.x86.avx2.vmovadvisew.store.256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 16)
  _mm256_vmovadvisew_store_avx_epi8(a, m, 16);
}

void test_mm256_vmemadvise_avx_epi8(__m256i const *a) {
  // CHECK-LABEL: test_mm256_vmemadvise_avx_epi8
  // CHECK: call void @llvm.x86.avx2.vmemadvise.256(i8* %{{.*}}, i8 16)
  _mm256_vmemadvise_avx_epi8(a, 16);
}
