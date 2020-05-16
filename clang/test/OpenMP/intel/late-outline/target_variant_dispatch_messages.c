// INTEL_COLLAB

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -verify \
//RUN:  -emit-llvm -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -Werror -Wsource-uses-openmp -o - %s

// Again with C++
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -x c++ -verify \
//RUN:  -emit-llvm -disable-llvm-passes -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -Werror -Wsource-uses-openmp -o - %s

int __attribute__((nothrow)) foo_gpu_four(float* p1, double* p2,
                                          void* interop_obj) { return 456; }

int __attribute__((nothrow)) foo_gpu_five(int* p1, double* p2) { return 333; }

int __attribute__((nothrow)) foo_gpu_six(float* p1, int* p2,
                                         void* interop_obj) { return 456; }

// expected-error@+1 {{incompatible with type 'int (float *, double *)}}
#pragma omp declare variant(foo_gpu_four) match(device={arch(gen)})
int __attribute__((nothrow)) foo(float* p1, double* p2) { return 123; }

// expected-error@+1 {{incompatible with type 'int (float *, double *)}}
#pragma omp declare variant(foo_gpu_five) \
             match(construct={target variant dispatch}, device={arch(gen)})
int __attribute__((nothrow)) foo_another(float* p1, double* p2) { return 123; }

// expected-error@+1 {{incompatible with type 'int (float *, double *)}}
#pragma omp declare variant(foo_gpu_six) \
             match(construct={target variant dispatch}, device={arch(gen)})
int __attribute__((nothrow)) foo_more(float* p1, double* p2) { return 123; }

// end INTEL_COLLAB
