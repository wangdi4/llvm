// INTEL_COLLAB
//
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -o - %s | FileCheck %s \
// RUN:  -check-prefixes CHECK,CHECKNOPAD

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -x c -o - %s | FileCheck %s \
// RUN:  -check-prefix CHECKC
//
// RUN: %clang_cc1 -triple mipsel-unknown-linux               \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 \
// RUN:  -fopenmp-late-outline -o - %s | FileCheck %s \
// RUN:  -check-prefixes CHECK,CHECKPAD
//
// Verify "openmp-variant" attribute string when adjust_args clause used
// with functions that have "hidden" sret pointer, 'this' pointer,
// expanded arguments (break structs into registers) and padding registers.

#include <omp.h>

#ifdef __cplusplus
template <typename T>
struct StructTemplate {
  T mbr1;
  T mbr2;
  T mbr3;
  T mbr4;
};

// Large struct return value, small and large struct arguments.
StructTemplate<long long> foo_v1(StructTemplate<int> alpha,
                            const StructTemplate<int> *AAA,
                            StructTemplate<int> beta,
                            const StructTemplate<int> *&BBB,
                            StructTemplate<int> gamma,
                            const StructTemplate<int> *CCC,
                            StructTemplate<int> delta) {
  StructTemplate<long long> StrReturn;
  StrReturn.mbr4 = 1;
  return StrReturn;
}

// Small struct return value, small and large struct arguments.
StructTemplate<int> foo_v2(StructTemplate<long long> alpha,
                            const StructTemplate<long long> *AAA,
                            StructTemplate<long long> beta,
                            const StructTemplate<long long> *BBB,
                            StructTemplate<long long> gamma,
                            const StructTemplate<long long> *CCC,
                            StructTemplate<long long> delta) {
  StructTemplate<int> StrReturn;
  StrReturn.mbr3 = 2;
  return StrReturn;
}

//CHECK: define{{.*}}foo1{{.*}}#[[FOO1BASE:[0-9]*]]
#pragma omp declare variant(foo_v1)                        \
   match(construct={dispatch}, device={arch(gen9)}),        \
   adjust_args(need_device_ptr:BBB,AAA)
StructTemplate<long long> foo1(StructTemplate<int> alpha,
                          const StructTemplate<int> *AAA,
                          StructTemplate<int> beta,
                          const StructTemplate<int> *&BBB,
                          StructTemplate<int> gamma,
                          const StructTemplate<int> *CCC,
                          StructTemplate<int> delta) {
  StructTemplate<long long> StrReturn;
  return StrReturn;
}

//CHECK: define{{.*}}foo2{{.*}}#[[FOO2BASE:[0-9]*]]
#pragma omp declare variant(foo_v2)                        \
   match(construct={dispatch}, device={arch(gen)}),        \
   adjust_args(need_device_ptr:BBB,CCC,AAA)
StructTemplate<int> foo2(StructTemplate<long long> alpha,
                         const StructTemplate<long long> *AAA,
                         StructTemplate<long long> beta,
                         const StructTemplate<long long> *BBB,
                         StructTemplate<long long> gamma,
                         const StructTemplate<long long> *CCC,
                         StructTemplate<long long> delta) {
  StructTemplate<int> StrReturn;
  StrReturn.mbr3 = 2;
  return StrReturn;
}

// Check static and non-static variant member functions with struct return
// types.
struct MyClass {
  static StructTemplate<long long> foo_mv1(float *AAA, float *&BBB);
  StructTemplate<long long> foo_mv2(float *AAA, float BBB, long long I);
  StructTemplate<long double> foo_mv3(StructTemplate<short> S1, float *AAA,
                                      long double *BBB);
  //CHECK: define{{.*}}mfoo1{{.*}}#[[MFOO1BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv1)                  \
     match(construct={dispatch}, device={arch(gen)})    \
     adjust_args(need_device_ptr:AAA,BBB)
  static StructTemplate<long long> mfoo1(float *AAA, float *&BBB) {
    StructTemplate<long long> StrResult;
    return StrResult;
  }

  //CHECK: define{{.*}}mfoo2{{.*}}#[[MFOO2BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv2)                       \
     match(construct={dispatch}, device={arch(gen9)}),       \
     adjust_args(need_device_ptr:AAA)
  StructTemplate<long long> mfoo2(float *AAA, float BBB, long long I) {
    StructTemplate<long long> StrResult;
    return StrResult;
  }

  //CHECK: define{{.*}}mfoo3{{.*}}#[[MFOO3BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv3)                           \
     adjust_args(need_device_ptr:AAA,BBB) \
     match(construct={dispatch}, device={arch(XeLP,XeHP)})
  StructTemplate<long double> mfoo3(StructTemplate<short> S1, float *AAA,
                                    long double *BBB) {
    StructTemplate<long double> StrResult;
    return StrResult;
  }
};


void func(float *A, float *B, int *I)
{
  float BB;
  long long II;
  MyClass mc;
  #pragma omp target
  {
    StructTemplate<short> S1;
    long double *BBB;

    #pragma omp dispatch
    mc.mfoo1(A, B);
    #pragma omp dispatch
    mc.mfoo2(A, BB, II);
    #pragma omp dispatch
    mc.mfoo3(S1, B, BBB);
  }
}

//CHECK:attributes #[[FOO1BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v1
// F <sret>, F,F <alpha>, T <AAA> , F,F <beta>, PTR_TO_PTR <&BBB>,
// F <gamma>, F <CCC>, F <delta>
//CHECKNOPAD-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,F,F,T,F,F,PTR_TO_PTR,F,F,F"
// F <sret>, F,F,F,F <alpha>, T <AAA>, F,F,F,F <beta>, PTR_TO_PTR <&BBB>,
// F,F,F,F <gamma>, F <CCC>, F,F,F,F <delta>
//CHECKPAD-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,F,F,F,F,T,F,F,F,F,PTR_TO_PTR,F,F,F,F,F,F,F,F,F"

//CHECK:attributes #[[FOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v2
// F <alpha>, T <AAA>, F <beta>, T <BBB>, F <gamma>, T <CCC>, F <delta>
//CHECKNOPAD-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,F,T,F,T,F"
// F <sret>, F <pad>, F,F,F,F,F,F,F,F <alpha>, T <AAA>, F <pad>,
// F,F,F,F,F,F,F,F <beta>,T <BBB>, F <pad>, F,F,F,F,F,F,F,F <gamma>,
// T <CCC>,F <pad>, F,F,F,F,F,F,F,F <delta>
//CHECKPAD-SAME:construct:dispatch;arch:gen;need_device_ptr:F,F,F,F,F,F,F,F,F,F,T,F,F,F,F,F,F,F,F,F,T,F,F,F,F,F,F,F,F,F,T,F,F,F,F,F,F,F,F,F"

//CHECK: attributes #[[MFOO1BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv1
// F <sret>, T <AAA>, PTR_TO_PTR <&BBB>
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,PTR_TO_PTR"

//CHECK: attributes #[[MFOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv2
// F <sret>, F <this>, T <AAA>, F <BBB>, F <I>
//CHECK-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,F,T,F,F"

//CHECK: attributes #[[MFOO3BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv3
//CHECKNOPAD-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:F,F,F,T,T"
// F <sret>, F <this>, F,F <S1, T <AAA>, T <BBB>
//CHECKPAD-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:F,F,F,F,T,T"
#else
typedef struct Foo {
  long double a;
  long double b;
} Foo;

typedef struct FooInt {
  int f1;
  int f2;
} FooInt;

Foo cfoo_v0(float *AAA, float *BBB) {Foo StrResult; return StrResult;}
void cfoo_v1(FooInt AAA, FooInt *BBB) {return;}
Foo cfoo_v2(FooInt AAA, Foo *BBB) {
  Foo StrResult;
  return StrResult;
}

//CHECKC: define{{.*}}cfoo1{{.*}}#[[CFOO1BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v0)                  \
   match(construct={dispatch}, device={arch(gen)})    \
   adjust_args(need_device_ptr:AAA,BBB)
Foo cfoo1(float *AAA, float *BBB) {Foo StrReturn; return StrReturn;}

//CHECKC: define{{.*}}cfoo2{{.*}}#[[CFOO2BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v1)                      \
   match(construct={dispatch}, device={arch(gen9)}),      \
   adjust_args(need_device_ptr:BBB)
void cfoo2(FooInt AAA, FooInt *BBB) {return;}

//CHECKC: define{{.*}}cfoo3{{.*}}#[[CFOO3BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v2)                           \
   adjust_args(need_device_ptr:BBB)                        \
   match(construct={dispatch}, device={arch(gen)})
Foo cfoo3(FooInt AAA, Foo *BBB) {Foo StrResult; return StrResult;}

//CHECKC:attributes #[[CFOO1BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v0
//CHECKC-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,T"

//CHECKC:attributes #[[CFOO2BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v1
//CHECKC-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,T"

//CHECKC: attributes #[[CFOO3BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v2
//CHECKC-SAME:construct:dispatch;arch:gen;need_device_ptr:F,F,T"
#endif
//
// end INTEL_COLLAB
