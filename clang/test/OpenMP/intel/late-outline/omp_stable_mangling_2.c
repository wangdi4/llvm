// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fintel-openmp-region  -o %t.bc %s
// RUN: %clang_cc1 -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fintel-openmp-region -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x_[0-9a-f]+]] = internal target_declare global i32 0,
// CHECK: [[VAR_Y:@y]] = hidden target_declare global float 0.000000e+00,
#pragma omp declare target
static int x;
float y;

void foo() {
// CHECK: [[VAR_Z:%z]] = alloca double, align 8
  double z;
}
#pragma omp end declare target
// end INTEL_COLLAB
