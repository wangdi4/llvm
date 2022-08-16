// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -I%S/Inputs -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-typed-clauses -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -opaque-pointers -I%S/Inputs -emit-llvm -o - -std=c++14 -fopenmp \
// RUN:  -fopenmp-typed-clauses -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -include-pch %t -verify \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// RUN: %clang_cc1 -opaque-pointers -I%S/Inputs -emit-llvm -o - -std=c++14 -fopenmp \
// RUN:  -fopenmp-typed-clauses -fopenmp-version=51 -fopenmp-late-outline \
// RUN:  -verify -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

#include <omp.h>

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

struct A {
  omp_interop_t M;
  static omp_interop_t SM;
  omp_interop_t member_func(float *Fp1, float *Fp2);
  static omp_interop_t static_member_func();
};

omp_interop_t A::member_func(float *Fp1, float *Fp2) {
  omp_interop_t I;
  //CHECK: [[I:%I.*]] = alloca ptr, align 8

  //CHECK: [[L1:%[0-9]+]] = load ptr, ptr @_ZN1A2SME
  //CHECK: [[CALL:%call.*]] = call {{.*}}static_member_func
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[L1]], ptr [[CALL]]
  #pragma omp dispatch interop(SM,static_member_func())
  bar2(Fp1, Fp2);

  //CHECK: [[M:%M.*]] = getelementptr inbounds %struct.A, ptr %this1, i32 0, i32 0
  //CHECK: [[L:%[0-9]+]] = load ptr, ptr [[M]]
  //CHECK: [[CALL:%call.*]] = call {{.*}}member_func
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[L]], ptr [[CALL]]
  #pragma omp dispatch interop(M,member_func(Fp1, Fp2))
  bar2(Fp1, Fp2);

  return I;
}

void foo1(float *Fp1, float *Fp2) {
  omp_interop_t I1, I2;
  //CHECK: [[FP1:%Fp1.*]] = alloca ptr, align 8
  //CHECK: [[FP2:%Fp2.*]] = alloca ptr, align 8
  //CHECK: [[I1:%I1.*]] = alloca ptr, align 8
  //CHECK: [[I2:%I2.*]] = alloca ptr, align 8
  //CHECK: [[FP1_MPT:%Fp1.map.*]] = alloca ptr, align 8
  //CHECK: [[FP2_MPT:%Fp2.map.*]] = alloca ptr, align 8
  //CHECK: [[I1_MPT:%I1.map.*]] = alloca ptr, align 8
  //CHECK: [[I2_MPT:%I2.map.*]] = alloca ptr, align 8
  //CHECK: [[SM_MPT:%SM.map.*]] = alloca ptr, align 8

  //CHECK: [[FP1_LOAD:%[0-9]+]] = load ptr, ptr [[FP1]]
  //CHECK: [[FP2_LOAD:%[0-9]+]] = load ptr, ptr [[FP2]]
  //CHECK: [[I1_LOAD:%[0-9]+]] = load ptr, ptr [[I1]]
  //CHECK: [[I2_LOAD:%[0-9]+]] = load ptr, ptr [[I2]]
  //CHECK: [[SM_LOAD:%[0-9]+]] = load ptr, ptr @_ZN1A2SME

  //CHECK: DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[FP1_LOAD]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[FP2_LOAD]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[I1_LOAD]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[I2_LOAD]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[SM_LOAD]]
  //CHECK: store ptr [[FP1_LOAD]], ptr [[FP1_MPT]]
  //CHECK: store ptr [[FP2_LOAD]], ptr [[FP2_MPT]]
  //CHECK: store ptr [[I1_LOAD]], ptr [[I1_MPT]]
  //CHECK: store ptr [[I2_LOAD]], ptr [[I2_MPT]]
  //CHECK: store ptr [[SM_LOAD]], ptr [[SM_MPT]]
  #pragma omp target
  {
    //CHECK: [ "DIR.OMP.DISPATCH"() ]
    #pragma omp dispatch
    bar1(Fp1, Fp2);

    //CHECK: [ "DIR.OMP.DISPATCH"() ]
    #pragma omp dispatch
    bar2(Fp1, Fp2);

    //CHECK: [[LD1:%[0-9]+]] = load ptr, ptr [[I1_MPT]]
    //CHECK: DIR.OMP.DISPATCH
    //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]]
    #pragma omp dispatch interop(I1)
    bar1(Fp1, Fp2);

    //CHECK: [[LD1:%[0-9]+]] = load ptr, ptr [[I1_MPT]]
    //CHECK: [[LD2:%[0-9]+]] = load ptr, ptr [[I2_MPT]]
    //CHECK: DIR.OMP.DISPATCH
    //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]], ptr [[LD2]]
    #pragma omp dispatch interop(I1, I2)
    bar2(Fp1, Fp2);

    //CHECK: [[LDSM:%[0-9]+]] = load ptr, ptr [[SM_MPT]]
    //CHECK: DIR.OMP.DISPATCH
    //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LDSM]]
    #pragma omp dispatch interop(A::SM)
    bar1(Fp1, Fp2);
  }
}

omp_interop_t func();
omp_interop_t iarr[10];
omp_interop_t *ip;

template <typename T>
void barTemp(T t, float *Fp1, float *Fp2) {
  T I1;
  //CHECK: [[T:%t.*]] = alloca ptr, align 8
  //CHECK: [[I1:%I1.*]] = alloca ptr, align 8

  //CHECK: [[LD1:%[0-9]+]] = load ptr, ptr [[I1]]
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]]
  #pragma omp dispatch interop(I1)
  bar1(Fp1, Fp2);

  //CHECK: [[LD1:%[0-9]+]] = load ptr, ptr [[I1]]
  //CHECK: [[LDT:%[0-9]+]] = load ptr, ptr [[T]]
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]], ptr [[LDT]]
  #pragma omp dispatch interop(I1,t)
  bar2(Fp1, Fp2);

  //CHECK: DIR.OMP.TASK{{.*}}"QUAL.OMP.IMPLICIT"
  //CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[I1]], ptr null, i32 1)
  //CHECK-NOT: INTEROP
  //CHECK: [[LD1:%[0-9]+]] = load ptr, ptr [[I1]]
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]]
  #pragma omp dispatch interop(I1) depend(inout:Fp1)
  bar1(Fp1, Fp2);

  //CHECK: [[CALL:%call.*]] = call {{.*}}func
  //CHECK: [[LD:%[0-9]+]] = load ptr, ptr getelementptr inbounds ([10 x ptr], ptr @iarr, i64 0, i64 3)
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[CALL]], ptr [[LD]]
  #pragma omp dispatch interop(func(),iarr[3])
  bar2(Fp1, Fp2);

  //CHECK: [[L1:%[0-9]+]] = load ptr, ptr @ip
  //CHECK: [[L2:%[0-9]+]] = load ptr, ptr [[L1]]
  //CHECK: [[L3:%[0-9]+]] = load ptr, ptr @ip
  //CHECK: [[AI:%arrayidx.*]] = getelementptr inbounds ptr, ptr [[L3]], i64 1
  //CHECK: [[L4:%[0-9]+]] = load ptr, ptr [[AI]]
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[L2]], ptr [[L4]]
  #pragma omp dispatch interop(*ip, ip[1])
  bar2(Fp1, Fp2);

  omp_interop_t &ir = iarr[6];
  #pragma omp interop init(target:ir)
  // CHECK: [[LD1:%[0-9]+]] = load ptr, ptr getelementptr (i8, ptr @iarr, i64 48)
  //CHECK: DIR.OMP.DISPATCH
  //CHECK-SAME: "QUAL.OMP.INTEROP"(ptr [[LD1]]
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
#endif
