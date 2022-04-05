// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -x c -o - %s | FileCheck %s -check-prefix CHECKC
//
// Verify function "openmp-variant" attribute string contains information
// specified by adjust_args and append_args clauses.

#include <omp.h>

#ifdef __cplusplus

void foo_v1(float *AAA, float *&BBB, int *I, omp_interop_t) {return;}
void foo_v2(float *&AAA, float *BBB, int *I, omp_interop_t) {return;}
void foo_v3(float *&AAA, float *&BBB, int *I, omp_interop_t, omp_interop_t) {
  return;
}
void foo_v4(double *AAA, float *BBB, int *&I, omp_interop_t) {return;}
void foo_v5(float *AAA, double *&BBB, int *I, omp_interop_t) {return;}
void foo_v6(double *AAA, double *BBB, int *I, omp_interop_t) {return;}

template <typename T>
struct SmallStruct {
  T real;
  T imag;
};

// Various small structs with by-value and by pointer arguments. Verify
// adjust_args string in openmp-variant metadata.
void foo_v7(SmallStruct<double> alpha, const SmallStruct<double> *AAA,
    SmallStruct<double> beta, const SmallStruct<double> *BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *CCC,
    SmallStruct<double> delta, void *interop_obj) {return;}

void foo_v7(SmallStruct<long> alpha, const SmallStruct<long> *AAA,
    SmallStruct<long> beta, const SmallStruct<long> *BBB,
    SmallStruct<long> gamma, const SmallStruct<long> *CCC,
    SmallStruct<long> delta, void *interop_obj) {return;}

void foo_v7(SmallStruct<int> alpha, const SmallStruct<int> *AAA,
    SmallStruct<int> beta, const SmallStruct<int> *BBB,
    SmallStruct<int> gamma, const SmallStruct<int> *CCC,
    SmallStruct<int> delta, void *interop_obj) {return;}

// Small struct arguments normally passed by-value or by pointer, now passed
// by-reference. Verify adjust_args string in openmp-variant metadata.
void foo_v7(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> &gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> &delta, void *interop_obj) {return;}

//CHECK: define{{.*}}foo1{{.*}}#[[FOO1BASE:[0-9]*]]
#pragma omp declare variant(foo_v1)                        \
   match(construct={dispatch}, device={arch(gen)})         \
   adjust_args(need_device_ptr:AAA,BBB) append_args(interop(target,targetsync))
void foo1(float *AAA, float *&BBB, int *I) {return;}

//CHECK: define{{.*}}foo2{{.*}}#[[FOO2BASE:[0-9]*]]
#pragma omp declare variant(foo_v2)                        \
   match(construct={dispatch}, device={arch(gen9)}),        \
   adjust_args(need_device_ptr:AAA) append_args(interop(targetsync,target))
void foo2(float *&AAA, float *BBB, int *I) {return;}

//CHECK: define{{.*}}foo3{{.*}}#[[FOO3BASE:[0-9]*]]
#pragma omp declare variant(foo_v3)                        \
   adjust_args(need_device_ptr:BBB,AAA) adjust_args(nothing:I) \
   append_args(interop(target),interop(targetsync)) \
   match(construct={dispatch}, device={arch(XeLP,XeHP)})
void foo3(float *&AAA, float *&BBB, int *I) {return;}

//CHECK: define{{.*}}foo4{{.*}}#[[FOO4BASE:[0-9]*]]
#pragma omp declare variant (foo_v4) \
    match(construct={dispatch}, device={arch(gen)}) \
    append_args(interop(targetsync)) adjust_args(need_device_ptr:I, BBB)
void foo4(double *AAA, float *BBB, int *&I);
void foo4(double *AAA, float *BBB, int *&I) {return;}

//CHECK: define{{.*}}foo5{{.*}}#[[FOO5BASE:[0-9]*]]
#pragma omp declare variant (foo_v5) \
    match(construct={dispatch}, device={arch(gen)}) \
    append_args(interop(targetsync)) adjust_args(need_device_ptr:BBB, AAA)
void foo5(float *AAA, double *&BBB, int *I);
void foo5(float *AAA, double *&BBB, int *I) {return;}

//CHECK: define{{.*}}foo6{{.*}}#[[FOO6BASE:[0-9]*]]
#pragma omp declare variant (foo_v6) \
    match(construct={dispatch}, device={arch(gen)}) \
    append_args(interop(targetsync)) adjust_args(need_device_ptr:BBB)
void foo6(double *AAA, double *BBB, int *I);
void foo6(double *AAA, double *BBB, int *I) {return;}

//CHECK: define{{.*}}foo7{{.*}}#[[FOO7BASE:[0-9]*]]
#pragma omp declare variant (foo_v7) \
    match(construct={dispatch}, device={arch(gen)}) \
    append_args(interop(targetsync)) \
    adjust_args(need_device_ptr:AAA,BBB,CCC) adjust_args(nothing:alpha, gamma)
void foo7(SmallStruct<double> alpha, const SmallStruct<double> *AAA,
    SmallStruct<double> beta, const SmallStruct<double> *BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *CCC,
    SmallStruct<double> delta) {return;}

//CHECK: define{{.*}}foo8{{.*}}#[[FOO8BASE:[0-9]*]]
#pragma omp declare variant (foo_v7) \
    match(construct={dispatch}, device={arch(gen9)}) \
    append_args(interop(targetsync)) \
    adjust_args(need_device_ptr:AAA,BBB,CCC) adjust_args(nothing:alpha, gamma)
void foo8(SmallStruct<long> alpha, const SmallStruct<long> *AAA,
    SmallStruct<long> beta, const SmallStruct<long> *BBB,
    SmallStruct<long> gamma, const SmallStruct<long> *CCC,
    SmallStruct<long> delta) {return;}

//CHECK: define{{.*}}foo9{{.*}}#[[FOO9BASE:[0-9]*]]
#pragma omp declare variant (foo_v7) \
    match(construct={dispatch}, device={arch(XeLP)}) \
    append_args(interop(targetsync)) \
    adjust_args(need_device_ptr:AAA,BBB,CCC) adjust_args(nothing:alpha, gamma)
void foo9(SmallStruct<int> alpha, const SmallStruct<int> *AAA,
    SmallStruct<int> beta, const SmallStruct<int> *BBB,
    SmallStruct<int> gamma, const SmallStruct<int> *CCC,
    SmallStruct<int> delta) {return;}

//CHECK: define{{.*}}foo10{{.*}}#[[FOO10BASE:[0-9]*]]
#pragma omp declare variant (foo_v7) \
    match(construct={dispatch}, device={arch(XeHP)}) \
    append_args(interop(targetsync)) \
    adjust_args(need_device_ptr:AAA,BBB,CCC) adjust_args(nothing:alpha, gamma)
void foo10(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> &gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> &delta) {return;}

void Foo_Var(float *AAA, float *BBB, omp_interop_t I1, omp_interop_t I2) {
  return;
}

//CHECK: define{{.*}}Foo_Var{{.*}}#
#pragma omp declare variant(Foo_Var) \
   match(construct={dispatch}, device={arch(XeHP)}) \
   adjust_args(need_device_ptr:AAA) adjust_args(nothing:BBB) \
   append_args(interop(target,targetsync), interop(targetsync,target))
template<typename T>
void Foo(T *AAA, T *BBB) {return;}

// Check static and non-static variant member functions.
struct MyClass {
  static void foo_mv1(float *AAA, float *&BBB, int *I, omp_interop_t) {return;}
  void foo_mv2(float *&AAA, float *BBB, int *I, omp_interop_t) {return;}
  void foo_mv3(float *&AAA, float *&BBB, int *I, omp_interop_t, omp_interop_t) {
    return;
  }
  //CHECK: define{{.*}}mfoo1{{.*}}#[[MFOO1BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv1)                  \
     match(construct={dispatch}, device={arch(gen)})    \
     adjust_args(need_device_ptr:AAA,BBB)               \
     append_args(interop(target,targetsync))
  static void mfoo1(float *AAA, float *&BBB, int *I) {return;}

  //CHECK: define{{.*}}mfoo2{{.*}}#[[MFOO2BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv2)                       \
     match(construct={dispatch}, device={arch(gen9)}),       \
     adjust_args(need_device_ptr:AAA) append_args(interop(targetsync,target))
  void mfoo2(float *&AAA, float *BBB, int *I) {return;}

  //CHECK: define{{.*}}mfoo3{{.*}}#[[MFOO3BASE:[0-9]*]]
  #pragma omp declare variant(foo_mv3)                           \
     adjust_args(need_device_ptr:AAA,BBB) adjust_args(nothing:I) \
     append_args(interop(target),interop(target))                \
     match(construct={dispatch}, device={arch(XeLP,XeHP)})
  void mfoo3(float *&AAA, float *&BBB, int *I) {return;}
};


void func(float *A, float *B, int *I)
{
  Foo(A, B);

  MyClass mc;
  #pragma omp target
  {
    #pragma omp dispatch
    mc.mfoo1(A, B, I);
    #pragma omp dispatch
    mc.mfoo2(A, B, I);
    #pragma omp dispatch
    mc.mfoo3(A, B, I);
  }
}

//CHECK:attributes #[[FOO1BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v1
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:T,PTR_TO_PTR,F,F;interop:target,targetsync"

//CHECK:attributes #[[FOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v2
//CHECK-SAME:construct:dispatch;arch:gen9;need_device_ptr:PTR_TO_PTR,F,F,F;interop:target,targetsync"

//CHECK: attributes #[[FOO3BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v3
//CHECK-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:PTR_TO_PTR,PTR_TO_PTR,F,F,F;interop:target;interop:targetsync"

//CHECK: attributes #[[FOO4BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v4
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,PTR_TO_PTR,F;interop:targetsync"
//CHECK: attributes #[[FOO5BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v5
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:T,PTR_TO_PTR,F,F;interop:targetsync"
//CHECK: attributes #[[FOO6BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v6
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,F,F;interop:targetsync"
//CHECK: attributes #[[FOO7BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v7
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,F,T,F,F,T,F,F,T,F,F,F;interop:targetsync"
//CHECK: attributes #[[FOO8BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v7
//CHECK-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,F,T,F,F,T,F,T,F,F;interop:targetsync"
//CHECK: attributes #[[FOO9BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v7
//CHECK-SAME:construct:dispatch;arch:XeLP;need_device_ptr:F,T,F,T,F,T,F,F;interop:targetsync"
//CHECK: attributes #[[FOO10BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v7
//CHECK-SAME:construct:dispatch;arch:XeHP;need_device_ptr:F,PTR_TO_PTR,F,F,PTR_TO_PTR,F,PTR_TO_PTR,F,F;interop:targetsync"

// Unlike normal functions, the attribute number for template functions varies
// from the number associated with the function definition. We can't verify the
// number is the same, but we can verify we have an appropriate variant string.
//CHECK:attributes #{{[0-9]+}} = {{.*}}"openmp-variant"=
//CHECK:name:{{.*}}Foo_Var
//CHECK-SAME:construct:dispatch;arch:XeHP;need_device_ptr:T,F,F,F;interop:target,targetsync;interop:target,targetsync"

//CHECK: attributes #[[MFOO1BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv1
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:T,PTR_TO_PTR,F,F;interop:target,targetsync"

//CHECK: attributes #[[MFOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv2
//CHECK-SAME:construct:dispatch;arch:gen9;need_device_ptr:F,PTR_TO_PTR,F,F,F;interop:target,targetsync"

//CHECK: attributes #[[MFOO3BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_mv3
//CHECK-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:F,PTR_TO_PTR,PTR_TO_PTR,F,F,F;interop:target;interop:target"
#else
void cfoo_v0(float *AAA, float *BBB) {return;}
void cfoo_v1(float *AAA, float *BBB, omp_interop_t I1) {return;}
void cfoo_v2(float *AAA, float *BBB, omp_interop_t I1, omp_interop_t I2) {
  return;
}

//CHECKC: define{{.*}}cfoo1{{.*}}#[[CFOO1BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v0)                  \
   match(construct={dispatch}, device={arch(gen)})    \
   adjust_args(need_device_ptr:AAA,BBB)
void cfoo1(float *AAA, float *BBB) {return;}

//CHECKC: define{{.*}}cfoo2{{.*}}#[[CFOO2BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v1)                      \
   match(construct={dispatch}, device={arch(gen9)}),      \
   adjust_args(need_device_ptr:AAA) append_args(interop(targetsync,target))
void cfoo2(float *AAA, float *BBB) {return;}

//CHECKC: define{{.*}}cfoo3{{.*}}#[[CFOO3BASE:[0-9]*]]
#pragma omp declare variant(cfoo_v2)                           \
   adjust_args(need_device_ptr:AAA,BBB)                        \
   append_args(interop(target),interop(targetsync))            \
   match(construct={dispatch}, device={arch(XeLP,XeHP)})
void cfoo3(float *AAA, float *BBB) {return;}

//CHECKC:attributes #[[CFOO1BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v0
//CHECKC-SAME:construct:dispatch;arch:gen;need_device_ptr:T,T"

//CHECKC:attributes #[[CFOO2BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v1
//CHECKC-SAME:construct:dispatch;arch:gen9;need_device_ptr:T,F,F;interop:target,targetsync"

//CHECKC: attributes #[[CFOO3BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v2
//CHECKC-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:T,T,F,F;interop:target;interop:targetsync"
#endif
// end INTEL_COLLAB
