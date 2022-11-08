// INTEL_COLLAB
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

void bar1(float *&F1, float *&F2);
void bar2(float* F1, float *F2, float *F3);
int bar_v3(float *&F1, ...);

#pragma omp declare variant(bar_v3) match(construct={dispatch})
int bar3(float *&FF1, ...);

//PRINT-LABEL: void foo1(
void foo1(float *Fp1, float *Fp2) {

  #pragma omp target
  {
    int r;
    //PRINT: #pragma omp dispatch need_device_ptr(1)
    #pragma omp dispatch need_device_ptr(1)
    bar1(Fp1, Fp2);
    //PRINT: #pragma omp dispatch need_device_ptr(1, 2)
    #pragma omp dispatch need_device_ptr(1, 2)
    bar2(Fp1, Fp2, Fp2);
    //PRINT: #pragma omp dispatch need_device_ptr(2, 1)
    #pragma omp dispatch need_device_ptr(2, 1)
    bar1(Fp1, Fp2);
    //PRINT: #pragma omp dispatch need_device_ptr(2, 1, 3)
    #pragma omp dispatch need_device_ptr(2, 1, 3)
    r = bar3(Fp1, Fp2, Fp1, Fp2);
  }
  //PRINT: #pragma omp dispatch need_device_ptr(3, 2, 1)
  #pragma omp dispatch need_device_ptr(3,2,1)
  bar2(Fp1, Fp2, Fp2);
  
  //PRINT: #pragma omp dispatch need_device_ptr(1)
  #pragma omp dispatch need_device_ptr(1)
  bar2(Fp1, Fp2, Fp2);

}

template <typename T>
void barTemp(T t, float *Fp1, float *Fp2) {
  T temp;

  //PRINT: #pragma omp target variant dispatch need_device_ptr(1) need_device_ptr(2)
  #pragma omp target variant dispatch need_device_ptr(1) need_device_ptr(2)
  bar1(Fp1, Fp2);

  //PRINT: #pragma omp target variant dispatch need_device_ptr(3) need_device_ptr(2) need_device_ptr(1)
  #pragma omp target variant dispatch need_device_ptr(3) need_device_ptr(2) need_device_ptr(1)
  bar2(Fp1, Fp2, Fp1);

  //PRINT: #pragma omp dispatch need_device_ptr(1, 3, 2)
  #pragma omp dispatch need_device_ptr(1, 3, 2)
  bar2(Fp1, Fp2, Fp1);

  //PRINT: #pragma omp dispatch need_device_ptr(2) need_device_ptr(3) need_device_ptr(1)
  #pragma omp dispatch need_device_ptr(2) need_device_ptr(3) need_device_ptr(1)
  bar2(Fp1, Fp2, Fp1);

#pragma omp target
  {
    int r;
    //PRINT: #pragma omp dispatch need_device_ptr(2) need_device_ptr(3) need_device_ptr(1)
    #pragma omp dispatch need_device_ptr(2) need_device_ptr(3) need_device_ptr(1, 4)
    r = bar3(Fp1, Fp2, Fp1, Fp1);
  }
}

void bar()
{
  float f1 = 1.0;
  float f2 = 2.0;
  int temp;
  barTemp(temp, &f1, &f2);
}
#endif // HEADER
// end INTEL_COLLAB
