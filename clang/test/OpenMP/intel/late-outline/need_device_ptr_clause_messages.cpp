// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -fopenmp-version=60 \
// RUN:    -std=c++11 -o - %s
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=51 -DNOINTEL \
// RUN:    -std=c++11 -o - %s

void bar_v1(float* F1, float *F2);
void bar_v2(float* F1, float *F2);
int bar_v3(float* F1, ...);
int bar_v4(int I1, double D1, float* F1, ...);

#pragma omp declare variant(bar_v1) match(construct={dispatch})
void bar1(float *FF1, float *FF2);

#pragma omp declare variant(bar_v2) match(construct={dispatch})
void bar2(float *FF1, float *FF2);

#pragma omp declare variant(bar_v3) match(construct={dispatch})
int bar3(float *FF1, ...);

#pragma omp declare variant(bar_v4) match(construct={dispatch})
int bar4(int I1, double D1, float* F1, ...);


void foo1(float *Fp1, float *Fp2) {
  #pragma omp target
  {
    int r;
    double d;

    #pragma omp dispatch
    r = bar3(Fp1, Fp2);  // okay, no clause
    #pragma omp dispatch
    bar2(Fp1, Fp2);  // okay, no clause

#ifdef NOINTEL
    //expected-warning@+1 {{extra tokens at the end of '#pragma omp dispatch' are ignored}}
    #pragma omp dispatch need_device_ptr(1)
    bar1(Fp1, Fp2);
#else

    int a;
    //expected-error@+1 {{integral constant expression must have integral or unscoped enumeration type, not 'double'}}
    #pragma omp dispatch need_device_ptr(1.0)
    bar1(Fp1, Fp2);
    
    //expected-error@+1 {{argument to 'need_device_ptr' clause must be a strictly positive integer value}}
    #pragma omp target variant dispatch need_device_ptr(0)
    bar1(Fp1, Fp2);

    //expected-error@+1 {{argument to 'need_device_ptr' clause must be a strictly positive integer value}}
    #pragma omp target variant dispatch need_device_ptr(-1)
    bar1(Fp1, Fp2);
    
    //expected-error@+1 {{'need_device_ptr' specifies argument '3' but call has only 2 arguments}}
    #pragma omp target variant dispatch need_device_ptr(3)
    bar1(Fp1, Fp2);

    //expected-error@+1 {{'need_device_ptr' specifies argument '3' but call has only 2 arguments}}
    #pragma omp target variant dispatch need_device_ptr(3)
    {
      bar1(Fp1, Fp2);
    }
    
    //expected-error@+1 {{argument 3 specified in 'need_device_ptr' clause must be a pointer}}
    #pragma omp dispatch need_device_ptr(3)
     bar3(Fp1, Fp2, r);
    
    //expected-error@+1 {{argument 1 specified in 'need_device_ptr' clause must be a pointer}}
    #pragma omp target variant dispatch need_device_ptr(1)
    {
      bar4(r, d, Fp1);
    }
    
    //expected-error@+1 {{argument 2 specified in 'need_device_ptr' clause must be a pointer}}
    #pragma omp dispatch need_device_ptr(2)
     bar4(r, d, Fp1);

    //expected-error@+1 {{'need_device_ptr' specifies argument '4' but call has only 3 arguments}}
    #pragma omp target variant dispatch need_device_ptr(4)
    r = bar3(Fp1, Fp2, Fp2);

    //expected-error@+1 {{'need_device_ptr' specifies argument '4' but call has only 1 argument}}
    #pragma omp target variant dispatch need_device_ptr(4)
    {
      r = bar3(Fp1);
    }
#endif
  }
}
// end INTEL_COLLAB
