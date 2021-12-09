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

struct MyClass {
  void foo_mv1(float *AAA, float *&BBB, int *I, omp_interop_t) {return;}
  void foo_mv2(float *&AAA, float *BBB, int *I, omp_interop_t) {return;}
  void foo_mv3(float *&AAA, float *&BBB, int *I, omp_interop_t, omp_interop_t) {
    return;
  }
  #pragma omp declare variant(foo_mv1)                  \
     match(construct={dispatch}, device={arch(gen)})    \
     adjust_args(need_device_ptr:AAA,BBB)               \
     append_args(interop(target,targetsync))
  void mfoo1(float *AAA, float *&BBB, int *I) {return;}

  #pragma omp declare variant(foo_mv2)                       \
     match(construct={dispatch}, device={arch(gen9)}),       \
     adjust_args(need_device_ptr:AAA) append_args(interop(targetsync,target))
  void mfoo2(float *&AAA, float *BBB, int *I) {return;}

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
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:T,PTR_TO_PTR,F;interop:target,targetsync"

//CHECK:attributes #[[FOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v2
//CHECK-SAME:construct:dispatch;arch:gen9;need_device_ptr:PTR_TO_PTR,F,F;interop:target,targetsync"

//CHECK: attributes #[[FOO3BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v3
//CHECK-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:PTR_TO_PTR,PTR_TO_PTR,F;interop:target;interop:targetsync"

//CHECK: attributes #[[FOO4BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v4
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,PTR_TO_PTR;interop:targetsync"
//CHECK: attributes #[[FOO5BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v5
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:T,PTR_TO_PTR,F;interop:targetsync"
//CHECK: attributes #[[FOO6BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v6
//CHECK-SAME:construct:dispatch;arch:gen;need_device_ptr:F,T,F;interop:targetsync"

// Unlike normal functions, the attribute number for template functions varies
// from the number associated with the function definition. We can't verify the
// number is the same, but we can verify we have an appropriate variant string.
//CHECK:attributes #{{[0-9]+}} = {{.*}}"openmp-variant"=
//CHECK:name:{{.*}}Foo_Var
//CHECK-SAME:construct:dispatch;arch:XeHP;need_device_ptr:T,F;interop:target,targetsync;interop:target,targetsync"
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
//CHECKC-SAME:construct:dispatch;arch:gen9;need_device_ptr:T,F;interop:target,targetsync"

//CHECKC: attributes #[[CFOO3BASE]] = {{.*}}"openmp-variant"=
//CHECKC-SAME:name:{{.*}}cfoo_v2
//CHECKC-SAME:construct:dispatch;arch:XeLP,XeHP;need_device_ptr:T,T;interop:target;interop:targetsync"
#endif

// end INTEL_COLLAB
