// INTEL_FEATURE_CSA
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility \
// RUN:   -fintel-openmp-region -triple csa \
// RUN:   | FileCheck %s
// REQUIRES: csa-registered-target

void bar(long long);

//CHECK-LABEL: foo
void foo(long long bound1, long long bound2)
{
  //CHECK: [[OMPIV:%.omp.iv.*]] = alloca i64,
  //CHECK: [[OMPLB:%.omp.lb.*]] = alloca i64,
  //CHECK: [[OMPUB:%.omp.ub.*]] = alloca i64,
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i64* [[OMPLB]]),
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(i64* [[OMPUB]])
  #pragma omp parallel
  {
    //CHECK-NOT: icmp slt i64

    //CHECK: store i64 0, i64* [[OMPLB]]
    //CHECK: store i64 {{.*}} [[OMPUB]]

    //CHECK: "DIR.OMP.LOOP"()
    //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[OMPLB]]),
    //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[OMPUB]])
    //CHECK: [[L9:%[0-9]+]] = load i64, i64* [[OMPLB]]
    //CHECK: store i64 [[L9]], i64* [[OMPIV]]

    //CHECK: [[L10:%[0-9]+]] = load i64, i64* [[OMPIV]]
    //CHECK: [[L11:%[0-9]+]] = load i64, i64* [[OMPUB]]
    //CHECK: icmp sle i64 [[L10]], [[L11]]

    #pragma omp for
    for (long long i = bound1; i < bound2; i++) {
      bar(i);
    }
  }
}

//CHECK-LABEL: footwo
void footwo(long long bound1, long long bound2)
{
  //CHECK: [[OMPIV:%.omp.iv.*]] = alloca i64,
  //CHECK: [[OMPLB:%.omp.lb.*]] = alloca i64,
  //CHECK: [[OMPUB:%.omp.ub.*]] = alloca i64,

  //CHECK-NOT: icmp slt i64

  //CHECK: store i64 0, i64* [[OMPLB]]
  //CHECK: store i64 {{.*}} [[OMPUB]]

  //CHECK: "DIR.OMP.PARALLEL.LOOP"()
  //CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[OMPLB]]),
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64* [[OMPUB]])

  //CHECK: [[L9:%[0-9]+]] = load i64, i64* [[OMPLB]]
  //CHECK: store i64 [[L9]], i64* [[OMPIV]]

  //CHECK: [[L10:%[0-9]+]] = load i64, i64* [[OMPIV]]
  //CHECK: [[L11:%[0-9]+]] = load i64, i64* [[OMPUB]]
  //CHECK: icmp sle i64 [[L10]], [[L11]]

  #pragma omp parallel for
  for (long long i = bound1; i < bound2; i++) {
    bar(i);
  }
}
// end INTEL_FEATURE_CSA
