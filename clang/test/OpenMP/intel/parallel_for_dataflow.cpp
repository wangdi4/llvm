// INTEL_FEATURE_CSA
// Test parallel for dataflow
// REQUIRES: csa-registered-target
// RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline -triple csa -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline -triple csa -emit-pch %s -o %t
// RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline -triple csa -include-pch %t -emit-llvm %s -o - | FileCheck %s
// expected-no-diagnostics

#ifndef HEADER
#define HEADER
void foo() {
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[OMP_IV2:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB2:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB2:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I2:%i.*]] = alloca i32,
  // CHECK: [[OMP_IV3:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB3:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB3:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I3:%i.*]] = alloca i32,
  // CHECK: [[OMP_IV3:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB3:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB3:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I4:%i.*]] = alloca i32,
  // CHECK: [[OMP_IV3:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB3:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB3:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I5:%i.*]] = alloca i32,
  // CHECK: [[OMP_IV3:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB3:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB3:%.omp.ub.*]] = alloca i32,
  // CHECK: [[I6:%i.*]] = alloca i32,
  // CHECK: [[A:%a*]] = alloca i32, align 4
  // CHECK: [[B:%b*]] = alloca i32, align 4
  // CHECK: [[C:%c*]] = alloca i32, align 4
  // CHECK: [[CE1:%.capture_expr.*]] = alloca i32, align 4
  // CHECK: [[CE2:%.capture_expr.*]] = alloca i32, align 4
  // CHECK: [[CE3:%.capture_expr.*]] = alloca i32, align 4
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 4
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
#pragma omp parallel for dataflow(num_workers(4))
  for (int i = 0; i < 16; ++i) {
  }
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 5
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I2]])
#pragma omp parallel for dataflow(static(5))
  for (int i = 0; i < 16; ++i) {
  }
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.PIPELINE"(i32 6
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I3]])
#pragma omp parallel for dataflow(pipeline(6))
  for (int i = 0; i < 16; ++i) {
  }
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 3
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 7
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I4]])
#pragma omp parallel for dataflow(static(3),num_workers(7))
  for (int i = 0; i < 16; ++i) {
  }
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 7
  // CHECK-SAME: "QUAL.OMP.PIPELINE"(i32 9
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I5]])
#pragma omp parallel for dataflow(pipeline(9),num_workers(7))
  for (int i = 0; i < 16; ++i) {
  }
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 12
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 7
  // CHECK-SAME: "QUAL.OMP.PIPELINE"(i32 9
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I6]])
#pragma omp parallel for dataflow(pipeline(9),num_workers(7),static(12))
  for (int i = 0; i < 16; ++i) {
  }
  int a = 3, b = 4, c = 5;
  // CHECK: store i32 3, i32* [[A]], align 4
  // CHECK: store i32 4, i32* [[B]], align 4
  // CHECK: store i32 5, i32* [[C]], align 4
  // CHECK: [[NUM1:%[0-9]+]] = load i32, i32* [[C]], align 4
  // CHECK: store i32 [[NUM1]], i32* [[CE1]], align 4
  // CHECK: [[NUM2:%[0-9]+]] = load i32, i32* [[B]], align 4
  // CHECK: store i32 [[NUM2]], i32* [[CE2]], align 4
  // CHECK: [[NUM3:%[0-9]+]] = load i32, i32* [[A]], align 4
  // CHECK: store i32 [[NUM3]], i32* [[CE3]], align 4
  // CHECK: store i32 0, i32* [[LB1:%.omp.lb[0-9]+]], align 4
  // CHECK: store i32 15, i32* [[UB1:%.omp.ub[0-9]+]], align 4
  // CHECK: [[NUM4:%[0-9]+]] = load i32, i32* [[CE1]], align 4
  // CHECK: [[NUM5:%[0-9]+]] = load i32, i32* [[CE2]], align 4
  // CHECK: [[NUM6:%[0-9]+]] = load i32, i32* [[CE3]], align 4
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"(),
  // CHECK-SAME: "QUAL.OMP.SCHEDULE.STATIC"(i32 [[NUM4]]
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 [[NUM5]]
  // CHECK-SAME: "QUAL.OMP.PIPELINE"(i32 [[NUM6]]
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[CE1]]
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[CE2]]
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[CE3]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LB1]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32*
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[UB1]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32*
#pragma omp parallel for dataflow(num_workers(b),static(c),pipeline(a))
  for (int i = 0; i < 16; ++i) {
  }
}
#endif
// end INTEL_FEATURE_CSA
