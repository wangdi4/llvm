// RUN: %clang_cc1 %s -ffreestanding -ffast-math -triple i386-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefix=CKFMATH %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffast-math -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefix=CKFMATH %s

// Include the metaheader that includes all intel intrinsic headers.
#include <immintrin.h>

// Make sure that the llvm IR for _mm_add_ps doesn't have fast math flags set
// CKFMATH: define{{.*}} <4 x float> @test_xmmintrin_no_reassoc
// CKFMATH: fadd <4 x float>
__m128 __attribute__((__target__("sse"))) test_xmmintrin_no_reassoc(__m128 __a, __m128 __b) {
  return _mm_add_ps(__a, __b);
}

// Make sure that the llvm IR for _mm_add_pd doesn't have fast math flags set
// CKFMATH: define{{.*}} <2 x double> @test_emmintrin_no_reassoc
// CKFMATH: fadd <2 x double>
__m128d __attribute__((__target__("sse2"))) test_emmintrin_no_reassoc(__m128d __a, __m128d __b) {
  return _mm_add_pd(__a, __b);
}

// Make sure that the llvm IR for _mm256_add_ps doesn't have fast math flags set
// This intrinsic comes from avxintrin.h, and so is checking that
// changes in immintrin.h affect the files it includes as well.
// CKFMATH: define{{.*}} <8 x float> @test_mm256intrin_no_reassoc
// CKFMATH: fadd <8 x float>
__m256 __attribute__((__target__(("avx")))) test_mm256intrin_no_reassoc(__m256 __a, __m256 __b) {
  return _mm256_add_ps(__a, __b);
}

// Make sure that the llvm IR for _mm512_add_ps doesn't have fast math flags set
// This intrinsic comes from avxintrin.h
// CKFMATH: define{{.*}} <16 x float> @test_mm512intrin_no_reassoc
// CKFMATH: fadd <16 x float>
__m512 __attribute__((__target__(("avx512f")))) test_mm512intrin_no_reassoc(__m512 __a, __m512 __b) {
  return _mm512_add_ps(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// CKFMATH: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
double test_fast(double __a, double __b) {
  return __a + __b;
}
