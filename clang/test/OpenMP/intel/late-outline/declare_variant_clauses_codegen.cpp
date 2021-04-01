// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -emit-llvm -disable-llvm-passes                      \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -o - %s | FileCheck %s

// Verify function "openmp-variant" attribute string contains information
// specified by adjust_args and append_args clauses.

void foo_v1(float *AAA, float *&BBB, int *I) {return;}
void foo_v2(float *&AAA, float *BBB, int *I) {return;}
void foo_v3(float *&AAA, float *&BBB, int *I) {return;}

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
   adjust_args(need_device_ptr:AAA,BBB) adjust_args(nothing:I) \
   append_args(interop(target),interop(target)) \
   match(construct={dispatch}, device={arch(XeLP,XeHP)})
void foo3(float *&AAA, float *&BBB, int *I) {return;}

void Foo_Var(float *AAA, float *BBB) {return;}

//CHECK: define{{.*}}Foo_Var{{.*}}#
#pragma omp declare variant(Foo_Var) \
   match(construct={dispatch}, device={arch(XeHP)}) \
   adjust_args(need_device_ptr:AAA) adjust_args(nothing:BBB) \
   append_args(interop(target,targetsync), interop(targetsync,target))
template<typename T>
void Foo(T *AAA, T *BBB) {return;}

void func()
{
  float *A;
  float *B;

  Foo(A, B);
}

//CHECK:attributes #[[FOO1BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v1
//CHECK-SAME:construct:;arch:gen;need_device_ptr:T,PTR_TO_PTR,F;interop:target,targetsync;"

//CHECK:attributes #[[FOO2BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v2
//CHECK-SAME:construct:;arch:gen9;need_device_ptr:PTR_TO_PTR,F,F;interop:target,targetsync;"

//CHECK: attributes #[[FOO3BASE]] = {{.*}}"openmp-variant"=
//CHECK-SAME:name:{{.*}}foo_v3
//CHECK-SAME:construct:;arch:XeLP,XeHP;need_device_ptr:PTR_TO_PTR,PTR_TO_PTR,F;interop:target;interop:target;"

// Unlike normal functions, the attribute number for template functions varies
// from the number associated with the function definition. We can't verify the
// number is the same, but we can verify we have an appropriate variant string.
//CHECK:attributes #{{[0-9]+}} = {{.*}}"openmp-variant"=
//CHECK:name:{{.*}}Foo_Var
//CHECK-SAME:construct:;arch:XeHP;need_device_ptr:T,F;interop:target,targetsync;interop:target,targetsync;"
// end INTEL_COLLAB
