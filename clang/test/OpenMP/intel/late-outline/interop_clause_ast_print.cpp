// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -fsyntax-only -verify %s

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -ast-print %s                               \
// RUN:   | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -emit-pch -o %t %s

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -include-pch %t -ast-print %s               \
// RUN:   | FileCheck %s --check-prefix=PRINT

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

typedef void *omp_interop_t;

void bar_v1(float* F1, float *F2, omp_interop_t);
void bar_v2(float* F1, float *F2, omp_interop_t, omp_interop_t);

// One appended interop variable.
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(target,targetsync))
void bar1(float *FF1, float *FF2);

// Two appended interop variables.
#pragma omp declare variant(bar_v2) match(construct={dispatch}) \
                        append_args(interop(targetsync), interop(target))
void bar2(float *FF1, float *FF2);

// Two appended interop variables - multiple variants for a single function.
#pragma omp declare variant(bar_v2)                                      \
                        match(construct={dispatch}, device={arch(gen)})   \
                        append_args(interop(targetsync), interop(target))
void bar1(float *FF1, float *FF2);

//PRINT-LABEL: void foo1(
void foo1(float *Fp1, float *Fp2) {
  omp_interop_t I1, I2;

  #pragma omp target
  {
    //PRINT: #pragma omp dispatch interop(I1)
    #pragma omp dispatch interop(I1)
    bar1(Fp1, Fp2);
    //PRINT: #pragma omp dispatch interop(I1,I2)
    #pragma omp dispatch interop(I1, I2)
    bar2(Fp1, Fp2);
    //PRINT: #pragma omp dispatch interop(I2,I1)
    #pragma omp dispatch interop(I2, I1)
    bar1(Fp1, Fp2);
  }
}

template <typename T>
void barTemp(T t, float *Fp1, float *Fp2) {
  T I1;
  //PRINT: #pragma omp dispatch interop(I1)
  #pragma omp dispatch interop(I1)
  bar1(Fp1, Fp2);
  //PRINT: #pragma omp dispatch interop(I1,t)
  #pragma omp dispatch interop(I1,t)
  bar2(Fp1, Fp2);
}

void bar()
{
  float f1 = 1.0;
  float f2 = 2.0;
  omp_interop_t Ivar;
  barTemp(Ivar, &f1, &f2);
}

#endif // HEADER
