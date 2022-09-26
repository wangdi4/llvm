// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -fopenmp-version=60 \
// RUN:    -std=c++11 -o - %s
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -DNOINTEL \
// RUN:    -std=c++11 -o - %s

typedef void *omp_interop_t;

void bar_v1(float* F1, float *F2, omp_interop_t);
void bar_v2(float* F1, float *F2, omp_interop_t, omp_interop_t);

// One appended interop variable.
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(target))
void bar1(float *FF1, float *FF2);

// Two appended interop variables.
#pragma omp declare variant(bar_v2) match(construct={dispatch}) append_args(interop(target), interop(target))
void bar2(float *FF1, float *FF2);


omp_interop_t func();

void foo1(float *Fp1, float *Fp2) {
  omp_interop_t I1, I2;
  #pragma omp target
  {
    #pragma omp dispatch
    bar1(Fp1, Fp2);  // okay, no clause
    #pragma omp dispatch
    bar2(Fp1, Fp2);  // okay, no clause

#ifdef NOINTEL
    //expected-warning@+1 {{extra tokens at the end of '#pragma omp dispatch' are ignored}}
    #pragma omp dispatch interop(I1)
    bar1(Fp1, Fp2);
#else
    int a;
    //expected-error@+1 {{'interop' clause expression must be of type 'omp_interop_t'}}
    #pragma omp dispatch interop(a)
    bar1(Fp1, Fp2);

    //expected-error@+1 {{expected expression}}
    #pragma omp dispatch interop()
    bar1(Fp1, Fp2);

    //expected-error@+1 {{directive '#pragma omp dispatch' cannot contain more than one 'interop' clause}}
    #pragma omp dispatch interop(I1) interop(I2)
    bar1(Fp1, Fp2);
#endif
  }
}
// end INTEL_COLLAB
