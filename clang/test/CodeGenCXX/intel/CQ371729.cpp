// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -std=c++11 -DPP1 %s | FileCheck --check-prefix=CHECK-PP1 %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -std=c++11 -DPP2 %s | FileCheck --check-prefix=CHECK-PP2 %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -DABI1 --gnu_fabi_version=1 %s | FileCheck --check-prefix=CHECK-ABI1 %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -DABI2 --gnu_fabi_version=2 %s | FileCheck --check-prefix=CHECK-ABI2 %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -std=c++11 -DVAR %s | FileCheck --check-prefix=CHECK-VAR %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -DSPEC %s | FileCheck --check-prefix=CHECK-SPEC %s
// RUN: %clang_cc1 %s -DVEC -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility | FileCheck %s
// RUN: %clang_cc1 %s -DVEC -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility --gnu_mangling_for_simd_types | FileCheck %s --check-prefix=CHECK-GNU
// RUN: %clang_cc1 %s -DVEC -ffreestanding -triple x86_64-unknown-linux-gnu -emit-llvm -o - -fintel-compatibility --gnu_mangling_for_simd_types --gnu_fabi_version=4 | FileCheck %s --check-prefix=CHECK-ABI4


#ifdef PP1
template<typename T>
auto f(T t) -> decltype(++t, 0)
{
  ++t;
  return 0;
}

template <class T>
struct A
{
  T operator[](int) const { return 0; }
};

template< typename T >
void g(const A<T> &a, decltype(a[0]) t) { }

int main()
{
  f((int*)0);

  A<int> a;
  g(a,1);
}

// CHECK-PP1-DAG: _Z1fIPiEDTcmpp_fp_Li0EET_
// CHECK-PP1-DAG: _Z1gIiEvRK1AIT_EDTixfL0p_Li0EE

#elif defined(PP2)
int i;
template<typename T>
auto f(T t) -> decltype(static_cast<T>(t++), 0) { t++; return 0; }

template <class T> auto f2 (T t) -> decltype(++t) { return i; }
template <class T> auto f4 (T t) -> decltype(--t) { return i; }


int main3()
{
  f((int*)0);
  f2(0);
  f4(0);
  return 0;
}

// CHECK-PP2-DAG: _Z1fIPiEDTcmscT_ppfp_Li0EES1_
// CHECK-PP2-DAG: _Z2f2IiEDTpp_fp_ET_
// CHECK-PP2-DAG: _Z2f4IiEDTmm_fp_ET_
#elif defined(ABI1)

extern "C" void Foo ();
namespace NMS
{
  extern "C" int V;
}

template <void (*)()> struct S {};
template <int *> struct T {};

void f (S<Foo>){}

void g (T<&NMS::V>){}

// CHECK-ABI1-DAG: _Z1f1SIXadL3FooEEE
// CHECK-ABI1-DAG: _Z1g1TIXadL_ZN3NMS1VEEEE

extern int N;
template <int &> struct R {};
void n (R<N>) {}
// CHECK-ABI1-DAG: _Z1n1RIXadL_Z1NEEE
#elif defined(ABI2)
extern int N;
template <int &> struct S {};
void n (S<N>) {}
// CHECK-ABI2-DAG: _Z1n1SILZ1NEE

void foo(char);
template<void (&)(char)> struct CB {};

void g(CB<foo> i) {}

// CHECK-ABI2-DAG: _Z1g2CBILZ3foocEE

#elif defined(VAR)
template<typename... Args>
class tuple {};

void f_none(tuple<>) {}
void f_one(tuple<int>) {}
void f_two(tuple<int, float>) {}
void f_nested(tuple<int, tuple<double, char>, float>) { }


// CHECK-VAR-DAG: _Z6f_none5tupleIIEE
// CHECK-VAR-DAG: _Z5f_one5tupleIIiEE
// CHECK-VAR-DAG: _Z5f_two5tupleIIifEE
// CHECK-VAR-DAG: _Z8f_nested5tupleIIiS_IIdcEEfEE

#elif defined(SPEC)
template<class T>
static void f1 (T) { }

template<>
void f1<float> (float) { }

template<class T>
void f2 (T) { }

template<>
void f2<float> (float) { }

void instantiator ()
{
  f1(0);

  f2(0);
}
// CHECK-SPEC: _Z2f1IiEvT_
#elif defined(VEC)
#include <immintrin.h>
void TestFunction1(__m64 x) { return; }
void TestFunction2(__m128 x) { return; }
void TestFunction3(__m128i x) { return; }
void TestFunction4(__m128d d) { return; }
void TestFunction5(int, __m128, __m128i, __m64,
                   float, __m128d, __m128i, __m128, __m128, __m64) { return; }
void *vp[5];
int main() { //lines below are useful to observe generated names in assembly text
  vp[0] = (void *)TestFunction1;
  vp[1] = (void *)TestFunction2;
  vp[2] = (void *)TestFunction3;
  vp[3] = (void *)TestFunction4;
  vp[4] = (void *)TestFunction5;
  return 0;
}

// CHECK-DAG: _Z13TestFunction1Dv1_x
// CHECK-DAG: _Z13TestFunction2Dv4_f
// CHECK-DAG: _Z13TestFunction3Dv2_x
// CHECK-DAG: _Z13TestFunction4Dv2_d
// CHECK-DAG: _Z13TestFunction5iDv4_fDv2_xDv1_xfDv2_dS0_S_S_S1_
// CHECK-GNU-DAG: _Z13TestFunction1U8__vectori
// CHECK-GNU-DAG: _Z13TestFunction2U8__vectorf
// CHECK-GNU-DAG: _Z13TestFunction3U8__vectorx
// CHECK-GNU-DAG: _Z13TestFunction4U8__vectord
// CHECK-GNU-DAG: _Z13TestFunction5iU8__vectorfU8__vectorxU8__vectorifU8__vectordS0_S_S_S1_
// CHECK-ABI4-DAG: _Z13TestFunction1Dv2_i
// CHECK-ABI4-DAG: _Z13TestFunction2Dv4_f
// CHECK-ABI4-DAG: _Z13TestFunction3Dv2_x
// CHECK-ABI4-DAG: _Z13TestFunction4Dv2_d
// CHECK-ABI4-DAG: _Z13TestFunction5iDv4_fDv2_xDv2_ifDv2_dS0_S_S_S1_
#endif
