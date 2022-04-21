// INTEL_COLLAB
// REQUIRES: clang-target-64-bits
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fopenmp-late-outline  -o %t.bc %s
// RUN: %clang_cc1 -no-opaque-pointers -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fopenmp-late-outline -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x]] = internal target_declare global i32 0,
// CHECK: @._ZL1x_[[HASH:[a-f0-9]+]].ref = internal constant i32* @x
// CHECK: [[VAR_Y:@foo.y]] = internal global float 0.000000e+00,
#pragma omp declare target
static int x;
#pragma omp end declare target

void foo() {
 static float y;

// CHECK: [[VAR_Z:%z]] = alloca i32, align 4
 int z;
// CHECK: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]])
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* [[VAR_X]], i32* [[VAR_X]], i64 4, i64 35
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(float* [[VAR_Y]], float* [[VAR_Y]], i64 4, i64 35
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[VAR_Z]]) ]
#pragma omp target map(x, y)
  {
    x = 5;
    y = 9.0f;
    z = 8;
  }
}

// CHECK: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"_ZL1x_[[HASH]]", i32 0, i32 0, i32* @x}
// end INTEL_COLLAB
