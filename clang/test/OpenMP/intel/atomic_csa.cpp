// INTEL_FEATURE_CSA
// This code was moved from atomic.cpp and openmp_codegen.cpp.
// TODO (vzakhari 11/14/2018): this is actually a hack that we do not use
//       #if guard above.  We have to live with this, until driver
//       starts defining the proper INTEL_FEATURE macros.
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
// RUN:   -fintel-openmp-region -triple csa \
// RUN:   | FileCheck %s
// REQUIRES: csa-registered-target

int i;
__int128 j;
//CHECK: define{{.*}}foo
void foo()
{
  #pragma omp target
  {
    //CHECK: atomicrmw add{{.*}}monotonic
    #pragma omp atomic
    i++;

    //CHECK-NOT: "DIR.OMP.ATOMIC"()
    #pragma omp atomic
    j++;
  }
}

void bar()
{
  // CHECK: [[N1_ADDR:%.+]] = alloca i64,
  // CHECK: [[N2_ADDR:%.+]] = alloca i64,
  long int n1, n2;
  n1 = 0;
// CHECK: atomicrmw add i64* [[N1_ADDR]], i64 1 monotonic
#pragma omp atomic
  ++n1;
// CHECK-NEXT: atomicrmw add i64* [[N1_ADDR]], i64 1 monotonic
#pragma omp atomic update
  ++n1;
// CHECK-NEXT: [[ALOAD:%.+]] = load atomic i64, i64* [[N1_ADDR]] monotonic
// CHECK-NEXT: store i64 [[ALOAD]], i64* [[N2_ADDR]], align 8
#pragma omp atomic read
  n2 = n1;
// CHECK-NEXT: store atomic i64 1, i64* [[N1_ADDR]] monotonic, align 8
#pragma omp atomic write
  n1 = 1;
// CHECK-NEXT: [[L45:%.+]] = atomicrmw add i64* [[N1_ADDR]], i64 1 monotonic
// CHECK-NEXT: [[A51:%.+]] = add nsw i64 [[L45]], 1
// CHECK-NEXT: store i64 [[A51]], i64* [[N2_ADDR]], align 8
#pragma omp atomic capture
  n2 = ++n1;
// CHECK-NEXT: atomicrmw add i64* [[N1_ADDR]], i64 1 seq_cst
// CHECK-NEXT: call void @__kmpc_flush
#pragma omp atomic seq_cst
  ++n1;
// CHECK-NEXT: atomicrmw add i64* [[N1_ADDR]], i64 1 seq_cst
// CHECK-NEXT: call void @__kmpc_flush
#pragma omp atomic seq_cst update
  ++n1;
// CHECK-NEXT: [[ALOAD52:%.+]] = load atomic i64, i64* [[N1_ADDR]] seq_cst
// CHECK-NEXT: call void @__kmpc_flush
// CHECK-NEXT: store i64 [[ALOAD52]], i64* [[N2_ADDR]], align 8
#pragma omp atomic read, seq_cst
  n2 = n1;
// CHECK-NEXT: store atomic i64 1, i64* [[N1_ADDR]] seq_cst, align 8
// CHECK-NEXT: call void @__kmpc_flush
#pragma omp atomic write seq_cst
  n1 = 1;
// CHECK-NEXT: [[L48:%.+]] = atomicrmw add i64* [[N1_ADDR]], i64 1 seq_cst
// CHECK-NEXT: [[ADD53:%.+]] = add nsw i64 [[L48]], 1
// CHECK-NEXT: store i64 [[ADD53]], i64* [[N2_ADDR]], align 8
// CHECK-NEXT: call void @__kmpc_flush
#pragma omp atomic seq_cst, capture
  n2 = ++n1;
}
// end INTEL_FEATURE_CSA
