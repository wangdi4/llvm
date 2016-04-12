// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility | FileCheck %s
// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility --gnu_mangling_for_simd_types | FileCheck %s --check-prefix=CHECK-GNU
// RUN: %clang_cc1 %s -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility --gnu_mangling_for_simd_types --gnu_fabi_version=4 | FileCheck %s --check-prefix=CHECK-ABI4

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

// CHECK-DAG: _Z13TestFunction15__m64
// CHECK-DAG: _Z13TestFunction26__m128
// CHECK-DAG: _Z13TestFunction37__m128i
// CHECK-DAG: _Z13TestFunction47__m128d
// CHECK-DAG: _Z13TestFunction5i6__m1287__m128i5__m64f7__m128dS0_S_S_S1_
// CHECK-DAG: _Z13TestFunction6U8__vectord
// CHECK-GNU-DAG: _Z13TestFunction1U8__vectori
// CHECK-GNU-DAG: _Z13TestFunction2U8__vectorf
// CHECK-GNU-DAG: _Z13TestFunction3U8__vectorx
// CHECK-GNU-DAG: _Z13TestFunction4U8__vectord
// CHECK-GNU-DAG: _Z13TestFunction5iU8__vectorfU8__vectorxU8__vectorifU8__vectordS0_S_S_S1_
// CHECK-GNU-DAG: _Z13TestFunction6U8__vectord
// CHECK-ABI4-DAG: _Z13TestFunction1Dv2_i
// CHECK-ABI4-DAG: _Z13TestFunction2Dv4_f
// CHECK-ABI4-DAG: _Z13TestFunction3Dv2_x
// CHECK-ABI4-DAG: _Z13TestFunction4Dv2_d
// CHECK-ABI4-DAG: _Z13TestFunction5iDv4_fDv2_xDv2_ifDv2_dS0_S_S_S1_
// CHECK-ABI4-DAG: _Z13TestFunction6Dv2_d

