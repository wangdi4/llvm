// RUN: %clang_cc1 %s -ffreestanding -ffast-math \
// RUN:     -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKFMATH %s
//
// RUN: %clang_cc1 %s -ffreestanding -ffp-exception-behavior=strict \
// RUN:      -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefixes=CKALL,CKSTRICT %s
//
#include <emmintrin.h>

// Make sure that the llvm IR for _mm_add_pd doesn't have any fast math flags
// CKALL: define{{.*}} <2 x double> @test_emmintrin_no_reassoc
// CKFMATH: fadd <2 x double>
// CKSTRICT: call <2 x double> @llvm.experimental.constrained.fadd.v2f64
__m128d test_emmintrin_no_reassoc(__m128d __a, __m128d __b) {
  return _mm_add_pd(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// For strict, make sure constrained intrinsics are still used
// CKALL: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
// CKSTRICT: call double @llvm.experimental.constrained.fadd.f64
double test_fast(double __a, double __b) {
  return __a + __b;
}
