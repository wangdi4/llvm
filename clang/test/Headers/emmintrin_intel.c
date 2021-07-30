// RUN: %clang_cc1 %s -ffreestanding -ffast-math -triple x86_64-unknown-unknown -emit-llvm -o - \
// RUN:     | FileCheck -check-prefix=CKFMATH %s
//
#include <emmintrin.h>

// Make sure that the llvm IR for _mm_add_pd doesn't have any fast math flags
// CKFMATH: define{{.*}} <2 x double> @test_emmintrin_no_reassoc
// CKFMATH: fadd <2 x double>
__m128d test_emmintrin_no_reassoc(__m128d __a, __m128d __b) {
  return _mm_add_pd(__a, __b);
}

// Make sure that all fast flags were restored outside of the include file.
// CKFMATH: define{{.*}} double @test_fast
// CKFMATH: fadd reassoc nnan ninf nsz arcp afn double
double test_fast(double __a, double __b) {
  return __a + __b;
}
