// RUN: %clang_cc1 %s -ffreestanding -ffast-math -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefix=CKFMATH %s
//
#include <xmmintrin.h>

// Make sure that the llvm IR for _mm_add_ps doesn't have fast math flags set
// CKFMATH: define{{.*}} <4 x float> @test_xmmintrin_no_reassoc
// CKFMATH: fadd <4 x float>
__m128 test_xmmintrin_no_reassoc(__m128 __a, __m128 __b) {
  return _mm_add_ps(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// CKFMATH: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
double test_fast(double __a, double __b) {
  return __a + __b;
}
