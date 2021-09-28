// RUN: %clang_cc1 %s -ffreestanding -ffast-math \
// RUN:     -triple i386-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKFMATH %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffast-math \
// RUN:     -triple i386-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKFMATH %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffp-exception-behavior=strict \
// RUN:     -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKSTRICT %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffp-exception-behavior=strict \
// RUN:     -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKSTRICT %s

// Include the metaheader that includes all intel intrinsic headers.
#include <immintrin.h>

// Make sure that the llvm IR for _mm_add_ps doesn't have fast math flags set
// CKALL: define{{.*}} <4 x float> @test_xmmintrin_no_reassoc
// CKFMATH: fadd <4 x float>
// CKSTRICT: call <4 x float> @llvm.experimental.constrained.fadd.v4f32
__m128 __attribute__((__target__("sse"))) test_xmmintrin_no_reassoc(__m128 __a, __m128 __b) {
  return _mm_add_ps(__a, __b);
}

// Make sure that the llvm IR for _mm_add_pd doesn't have fast math flags set
// CKALL: define{{.*}} <2 x double> @test_emmintrin_no_reassoc
// CKFMATH: fadd <2 x double>
// CKSTRICT: call <2 x double> @llvm.experimental.constrained.fadd.v2f64
__m128d __attribute__((__target__("sse2"))) test_emmintrin_no_reassoc(__m128d __a, __m128d __b) {
  return _mm_add_pd(__a, __b);
}

// Make sure that the llvm IR for _mm256_add_ps doesn't have fast math flags set
// This intrinsic comes from avxintrin.h, and so is checking that
// changes in immintrin.h affect the files it includes as well.
// CKALL: define{{.*}} <8 x float> @test_mm256intrin_no_reassoc
// CKFMATH: fadd <8 x float>
// CKSTRICT: call <8 x float> @llvm.experimental.constrained.fadd.v8f32
__m256 __attribute__((__target__(("avx")))) test_mm256intrin_no_reassoc(__m256 __a, __m256 __b) {
  return _mm256_add_ps(__a, __b);
}

// Make sure that the llvm IR for _mm512_add_ps doesn't have fast math flags set
// This intrinsic comes from avxintrin.h
// CKALL: define{{.*}} <16 x float> @test_mm512intrin_no_reassoc
// CKFMATH: fadd <16 x float>
// CKSTRICT: call <16 x float> @llvm.experimental.constrained.fadd.v16f32
__m512 __attribute__((__target__(("avx512f")))) test_mm512intrin_no_reassoc(__m512 __a, __m512 __b) {
  return _mm512_add_ps(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// For strict, make sure constrained intrinsics are still used
// CKALL: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
// CKSTRICT: call double @llvm.experimental.constrained.fadd.f64
double test_fast(double __a, double __b) {
  return __a + __b;
}
