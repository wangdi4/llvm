// REQUIRES: intel_feature_isa_avx_memadvise
// RUN: %clang_cc1 -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx512memadvise -target-feature +avx512vl -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <immintrin.h>
__m128i test_mm_vmovadvisew_loade_epi8(char const *a) {
  // CHECK-LABEL: test_mm_vmovadvisew_loade_epi8
  // CHECK: call <4 x i32> @llvm.x86.avx512.vmovadvisew.load.128(i8* %{{.*}}, i8 16)
  return _mm128_vmovadvisew_loade_epi8(a, 16);
}

void test_mm_vmovadvisew_storee_epi8(char *a, __m128i m) {
  // CHECK-LABEL: test_mm_vmovadvisew_storee_epi8
  // CHECK: call void @llvm.x86.avx512.vmovadvisew.store.128(i8* %{{.*}}, <4 x i32> %{{.*}}, i8 16)
  _mm128_vmovadvisew_storee_epi8(a, m, 16);
}

__m256i test_mm256_vmovadvisew_loade_epi8(char const *a) {
  // CHECK-LABEL: test_mm256_vmovadvisew_loade_epi8
  // CHECK: call <8 x i32> @llvm.x86.avx512.vmovadvisew.load.256(i8* %{{.*}}, i8 16)
  return _mm256_vmovadvisew_loade_epi8(a, 16);
}

void test_mm256_vmovadvisew_storee_epi8(char *a, __m256i m) {
  // CHECK-LABEL: test_mm256_vmovadvisew_storee_epi8
  // CHECK: call void @llvm.x86.avx512.vmovadvisew.store.256(i8* %{{.*}}, <8 x i32> %{{.*}}, i8 16)
  _mm256_vmovadvisew_storee_epi8(a, m, 16);
}

__m512i test_mm512_vmovadvisew_loade_epi8(char const *a) {
  // CHECK-LABEL: test_mm512_vmovadvisew_loade_epi8
  // CHECK: call <16 x i32> @llvm.x86.avx512.vmovadvisew.load.512(i8* %{{.*}}, i8 16)
  return _mm512_vmovadvisew_loade_epi8(a, 16);
}

void test_mm512_vmovadvisew_storee_epi8(char *a, __m512i m) {
  // CHECK-LABEL: test_mm512_vmovadvisew_storee_epi8
  // CHECK: call void @llvm.x86.avx512.vmovadvisew.store.512(i8* %{{.*}}, <16 x i32> %{{.*}}, i8 16)
  _mm512_vmovadvisew_storee_epi8(a, m, 16);
}
