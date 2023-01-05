// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes=CHECK,C

// Again with C++
//RUN: %clang_cc1 -x c++ -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -x c++ -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes=CHECK,CPP

//expected-no-diagnostics

int __attribute__((nothrow)) foo_gpu_three(float* p1, double* p2,
                                           void* interop_obj) { return 456; }

int __attribute__((nothrow)) foo_gpu_two(float* p1, double* p2) { return 333; }

#pragma omp declare variant(foo_gpu_three) \
             match(construct={target variant dispatch}, device={arch(gen)})
int __attribute__((nothrow)) foo(float* p1, double* p2) { return 123; }

#pragma omp declare variant(foo_gpu_two) \
             match(construct={target variant dispatch}, device={arch(gen)})
int __attribute__((nothrow)) foo_another(float* p1, double* p2) { return 123; }

void __attribute__((nothrow,noinline))  foo_gpu(void* interop) { }

#pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
void __attribute__((nothrow,noinline))  foo0() {}

float  *aaa;
double *bbb;

#ifdef __cplusplus
struct A {
  int member_gpu(float* p1, double* p2, void* interop_obj) { return 456; }
  #pragma omp declare variant(member_gpu) \
    match(construct={target variant dispatch}, device={arch(gen)})
  int member(float* p1, double* p2) { return 123; }
};
#endif

int main() {
  int rrr;

  #pragma omp target
  {
    //CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH{{.*}}
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CHECK: call{{.*}}foo{{.*}}[ "QUAL.OMP.DISPATCH.CALL"() ]
    rrr = foo(aaa, bbb);
    //CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"

    //CHECK: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CHECK: call{{.*}}foo_another{{.*}}[ "QUAL.OMP.DISPATCH.CALL"() ]
    rrr = foo_another(aaa, bbb);
    //CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"

#ifdef __cplusplus
    A avar;
    //CPP: [[T3:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CPP: call{{.*}}member{{.*}}[ "QUAL.OMP.DISPATCH.CALL"() ]
    rrr = avar.member(aaa, bbb);
    //CPP: region.exit(token [[T3]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"
#endif
  }
  #pragma omp target variant dispatch device(0) nowait
  {
    //CHECK: [[T4:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    //CHECK: call{{.*}}foo0{{.*}}[ "QUAL.OMP.DISPATCH.CALL"() ]
    foo0();
    //CHECK: region.exit(token [[T4]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"
  }
  return rrr;
}
// CPP: declare spir_func void @_Z4foo0v()
// C: declare spir_func void @foo0(...)
// end INTEL_COLLAB
