// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility | FileCheck %s --check-prefix=CHECK-ABI4

#include <immintrin.h>
void TestFunction1(__m64 x) { return; }
void TestFunction2(__m128 x) { return; }
void TestFunction3(__m128i x) { return; }
void TestFunction4(__m128d d) { return; }
void TestFunction5(int, __m128, __m128i, __m64,
                   float, __m128d, __m128i, __m128, __m128, __m64) { return; }
void TestFunction6(double __attribute__((vector_size(16)))) { return; }
void *vp[6];
int main() { //lines below are useful to observe generated names in assembly text
  vp[0] = (void *)TestFunction1;
  vp[1] = (void *)TestFunction2;
  vp[2] = (void *)TestFunction3;
  vp[3] = (void *)TestFunction4;
  vp[4] = (void *)TestFunction5;
  vp[5] = (void *)TestFunction6;
  return 0;
}

// FIXME: Clang/GCC seem to have different mangling for this function, we should
// fix this in Clang community to get this to be _Z13TestFunction1Dv2_i
// CHECK-ABI4-DAG: _Z13TestFunction1Dv1_x
// CHECK-ABI4-DAG: _Z13TestFunction2Dv4_f
// CHECK-ABI4-DAG: _Z13TestFunction3Dv2_x
// CHECK-ABI4-DAG: _Z13TestFunction4Dv2_d
// CHECK-ABI4-DAG: _Z13TestFunction5iDv4_fDv2_xDv1_xfDv2_dS0_S_S_S1_
// CHECK-ABI4-DAG: _Z13TestFunction6Dv2_d

