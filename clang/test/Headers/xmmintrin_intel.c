// RUN: %clang_cc1 %s -ffreestanding -ffast-math \
// RUN:     -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKFMATH %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffp-exception-behavior=strict \
// RUN:     -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKSTRICT %s
//
#include <xmmintrin.h>

// Make sure that the llvm IR for _mm_add_ps doesn't have fast math flags set
// For strict, make sure constrained intrinsics are still used
// CKALL: define{{.*}} <4 x float> @test_xmmintrin_no_reassoc
// CKFMATH: fadd <4 x float>
// CKSTRICT: call <4 x float> @llvm.experimental.constrained.fadd.v4f32
__m128 test_xmmintrin_no_reassoc(__m128 __a, __m128 __b) {
  return _mm_add_ps(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// For strict, make sure constrained intrinsics are still used
// CKALL: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
// CKSTRICT: call double @llvm.experimental.constrained.fadd.f64
double test_fast(double __a, double __b) {
  return __a + __b;
}
