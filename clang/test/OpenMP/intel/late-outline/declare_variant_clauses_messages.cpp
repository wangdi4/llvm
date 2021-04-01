// INTEL_COLLAB
// This test is going to be removed after upstreaming is complete.
//
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 \
// RUN:  -DOMP51 -std=c++11 -o - %s

// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=50 \
// RUN:  -DOMP50 -std=c++11 -o - %s

int Other;

void foo_v1(float *AAA, float *BBB, int *I) { return; }
void foo_v2(float *AAA, float *BBB, int *I) { return; }
void foo_v3(float *AAA, float *BBB, int *I) { return; }

#ifdef OMP51
// expected-error@+3 {{adjust_arg argument 'AAA' used in multiple clauses}}
#pragma omp declare variant(foo_v1)                          \
   match(construct={dispatch}, device={arch(arm)})           \
   adjust_args(need_device_ptr:AAA,BBB) adjust_args(need_device_ptr:AAA)

// expected-error@+3 {{adjust_arg argument 'AAA' used in multiple clauses}}
#pragma omp declare variant(foo_v1)                          \
   match(construct={dispatch}, device={arch(ppc)}),          \
   adjust_args(need_device_ptr:AAA) adjust_args(nothing:AAA)

// expected-error@+2 {{use of undeclared identifier 'J'}}
#pragma omp declare variant(foo_v1)                          \
   adjust_args(nothing:J)                                    \
   match(construct={dispatch}, device={arch(x86,x86_64)})

// expected-error@+2 {{expected reference to one of the parameters of function 'foo'}}
#pragma omp declare variant(foo_v3)                          \
   adjust_args(nothing:Other)                                \
   match(construct={dispatch}, device={arch(x86,x86_64)})

// expected-error@+2 {{'adjust_args' clause requires 'dispatch' context selector}}
#pragma omp declare variant(foo_v3)                          \
   adjust_args(nothing:BBB) match(construct={target}, device={arch(arm)})

// expected-error@+2 {{'adjust_args' clause requires 'dispatch' context selector}}
#pragma omp declare variant(foo_v3)                          \
   adjust_args(nothing:BBB) match(device={arch(ppc)})

// expected-error@+2 {{'append_args' clause requires 'dispatch' context selector}}
#pragma omp declare variant(foo_v2)                          \
   append_args(interop(target)) match(device={arch(ppc)})

// expected-error@+2 {{unexpected append-op in 'append_args' clause, expected 'interop'}}
#pragma omp declare variant(foo_v2)                          \
   append_args(inerop(target)) match(construct={dispatch}, device={arch(ppc)})
// expected-error@+2 {{expected interop type: 'target' and/or 'targetsync'}}
#pragma omp declare variant(foo_v2)                          \
   append_args(interop(taget)) match(construct={dispatch}, device={arch(ppc)})
// expected-error@+2 {{expected interop type: 'target' and/or 'targetsync'}}
#pragma omp declare variant(foo_v2)                          \
   append_args(interop()) match(construct={dispatch}, device={arch(ppc)})
// expected-warning@+2 {{interop type 'target' cannot be specified more than once}}
#pragma omp declare variant(foo_v2)                          \
   append_args(interop(target,target)) match(construct={dispatch}, device={arch(ppc)})

// expected-error@+5 {{directive '#pragma omp declare variant' cannot contain more than one 'append_args' clause}}
#pragma omp declare variant(foo_v1)                        \
   match(construct={dispatch}, device={arch(gen)})         \
   append_args(interop(target)) \
   adjust_args(need_device_ptr:AAA,BBB) \
   append_args(interop(targetsync))
#endif // OMP51
#ifdef OMP50
// expected-error@+2 {{expected 'match' clause on 'omp declare variant' directive}}
#pragma omp declare variant(foo_v1)                            \
   adjust_args(need_device_ptr:AAA) match(device={arch(arm)})
#endif // OMP50

void foo(float *AAA, float *BBB, int *I) { return; }

// end INTEL_COLLAB
