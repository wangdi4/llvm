// INTEL_COLLAB
// REQUIRES: clang-target-64-bits
// RUN: %clang_cc1 -opaque-pointers -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fopenmp-late-outline  -o %t.bc %s
// RUN: %clang_cc1 -opaque-pointers -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fopenmp-late-outline -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x]] = internal target_declare global i32 0,
// CHECK: @._ZL1x_[[HASH:[a-f0-9]+]].ref = internal constant ptr @x
// CHECK: [[VAR_Y:@y]] ={{ dso_local | }}target_declare global float 0.000000e+00,
#pragma omp declare target
static int x;
float y;

void foo() {
// CHECK: [[VAR_Z:%z]] = alloca double, align 8
  double z;
}
#pragma omp end declare target

// CHECK-DAG: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"_ZL1x_[[HASH]]", i32 0, i32 0, ptr @x}
// CHECK-DAG: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"_Z1y", i32 0, i32 1, ptr @y}
// end INTEL_COLLAB
