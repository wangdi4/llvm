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

struct A {
 omp_interop_t member;
 static omp_interop_t static_member;
 omp_interop_t member_func(float *Fp1, float *Fp2);
 static omp_interop_t static_member_func();
};

omp_interop_t A::member_func(float *Fp1, float *Fp2) {
  omp_interop_t I;
  //PRINT: #pragma omp dispatch interop(A::static_member,static_member_func())
  #pragma omp dispatch interop(A::static_member,static_member_func())
  bar2(Fp1, Fp2);

  //PRINT: #pragma omp dispatch interop(this->member_func(Fp1, Fp2))
  #pragma omp dispatch interop(member_func(Fp1, Fp2))
  bar1(Fp1, Fp2);

  return I;
}

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
  // PRINT: #pragma omp dispatch interop(A::static_member,A::static_member_func())
  #pragma omp dispatch interop(A::static_member,A::static_member_func())
  bar2(Fp1, Fp2);

  A avar;
  // PRINT: #pragma omp dispatch interop(avar.member,avar.member_func(Fp1, Fp2))
  #pragma omp dispatch interop(avar.member, avar.member_func(Fp1, Fp2))
  bar2(Fp1, Fp2);

}

omp_interop_t func();
omp_interop_t iarr[10];
omp_interop_t *ip;

template <typename T>
void barTemp(T t, float *Fp1, float *Fp2) {
  T I1;
  //PRINT: #pragma omp dispatch interop(I1)
  #pragma omp dispatch interop(I1)
  bar1(Fp1, Fp2);
  //PRINT: #pragma omp dispatch interop(I1,t)
  #pragma omp dispatch interop(I1,t)
  bar2(Fp1, Fp2);

  //PRINT: #pragma omp dispatch interop(func(),iarr[3])
  #pragma omp dispatch interop(func(),iarr[3])
  bar2(Fp1, Fp2);

  //PRINT: #pragma omp dispatch interop(*ip,ip[1])
  #pragma omp dispatch interop(*ip, ip[1])
  bar2(Fp1, Fp2);

  omp_interop_t &ir = iarr[6];
  #pragma omp interop init(target:ir)
  //PRINT: #pragma omp dispatch interop(ir)
  #pragma omp dispatch interop(ir)
  bar1(Fp1, Fp2);
}

void bar()
{
  float f1 = 1.0;
  float f2 = 2.0;
  omp_interop_t Ivar;
  barTemp(Ivar, &f1, &f2);
}

#endif // HEADER
// end INTEL_COLLAB
