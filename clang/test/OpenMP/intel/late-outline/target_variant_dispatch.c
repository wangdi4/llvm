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
//RUN:  | FileCheck %s

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
    //CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CHECK: call{{.*}}foo
    rrr = foo(aaa, bbb);
    //CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"

    //CHECK: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CHECK: call{{.*}}foo_another
    rrr = foo_another(aaa, bbb);
    //CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"

#ifdef __cplusplus
    A avar;
    //CPP: [[T3:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
    //CPP: call{{.*}}member
    rrr = avar.member(aaa, bbb);
    //CPP: region.exit(token [[T3]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"
#endif
  }
  return rrr;
}
// end INTEL_COLLAB
