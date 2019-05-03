// REQUIRES: intel_feature_isa_keylocker
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +keylocker -emit-llvm -o - -Wall -Werror | FileCheck %s
// RUN: %clang_cc1 %s -ffreestanding -triple=i386-unknown-unknown -target-feature +keylocker -emit-llvm -o - -Wall -Werror | FileCheck %s

#include <x86intrin.h>

void test_loadiwkey(unsigned int ctl, __m128i intkey, __m128i enkey_lo, __m128i enkey_hi) {
  //CHECK-LABEL: @test_loadiwkey
  //CHECK: @llvm.x86.loadiwkey
  _mm_loadiwkey(ctl, intkey, enkey_lo, enkey_hi);
}

unsigned int test_encodekey128_u32(unsigned int htype, __m128i key, void *h) {
  //CHECK-LABEL: @test_encodekey128_u32
  //CHECK: call { i32, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.encodekey128(i32 %{{.*}}, <2 x i64> %{{.*}})
  return _mm_encodekey128_u32(htype, key, h);
}

unsigned int test_encodekey256_u32(unsigned int htype, __m128i key_lo, __m128i key_hi, void *h) {
  //CHECK-LABEL: @test_encodekey256_u32
  //CHECK: call { i32, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.encodekey256(i32 %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_encodekey256_u32(htype, key_lo, key_hi, h);
}

__m128i test_mm_aesenc256kl_si128(__m128i data, const void *h) {
  //CHECK-LABEL: @test_mm_aesenc256kl_si128
  //CHECK: call <2 x i64> @llvm.x86.aesenc256kl(<2 x i64> %{{.*}}, i8* %{{.*}})
  return _mm_aesenc256kl_si128(data, h);
}

__m128i test_mm_aesdec256kl_si128(__m128i data, const void *h) {
  //CHECK-LABEL: @test_mm_aesdec256kl_si128
  //CHECK: call <2 x i64> @llvm.x86.aesdec256kl(<2 x i64> %{{.*}}, i8* %{{.*}})
  return _mm_aesdec256kl_si128(data, h);
}

__m128i test_mm_aesenc128kl_si128(__m128i data, const void *h) {
  //CHECK-LABEL: @test_mm_aesenc128kl_si128
  //CHECK: call <2 x i64> @llvm.x86.aesenc128kl(<2 x i64> %{{.*}}, i8* %{{.*}})
  return _mm_aesenc128kl_si128(data, h);
}

__m128i test_mm_aesdec128kl_si128(__m128i data, const void *h) {
  //CHECK-LABEL: @test_mm_aesdec128kl_si128
  //CHECK: call <2 x i64> @llvm.x86.aesdec128kl(<2 x i64> %{{.*}}, i8* %{{.*}})
  return _mm_aesdec128kl_si128(data, h);
}

unsigned char test__mm_aesencwide128kl(__m128i odata[8], const __m128i idata[8], const void* h) {
  //CHECK-LABEL: @test__mm_aesencwide128kl
  //CHECK: call { i8, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.aesencwide128kl(i8* %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}},      <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_aesencwide128kl(odata, idata, h);
}

unsigned char test__mm_aesdecwide128kl(__m128i odata[8], const __m128i idata[8], const void* h) {
  //CHECK-LABEL: @test__mm_aesdecwide128kl
  //CHECK: call { i8, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.aesdecwide128kl(i8* %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}},      <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_aesdecwide128kl(odata, idata, h);
}

unsigned char test__mm_aesencwide256kl(__m128i odata[8], const __m128i idata[8], const void* h) {
  //CHECK-LABEL: @test__mm_aesencwide256kl
  //CHECK: call { i8, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.aesencwide256kl(i8* %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}},      <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_aesencwide256kl(odata, idata, h);
}

unsigned char test__mm_aesdecwide256kl(__m128i odata[8], const __m128i idata[8], const void* h) {
  //CHECK-LABEL: @test__mm_aesdecwide256kl
  //CHECK: call { i8, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64>, <2 x i64> } @llvm.x86.aesdecwide256kl(i8* %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}},      <2 x i64> %{{.*}}, <2 x i64> %{{.*}}, <2 x i64> %{{.*}})
  return _mm_aesdecwide256kl(odata, idata, h);
}
