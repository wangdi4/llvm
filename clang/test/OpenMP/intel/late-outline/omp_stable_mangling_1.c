// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fintel-openmp-region  -o %t.bc %s
// RUN: %clang_cc1 -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fintel-openmp-region -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x_[0-9a-f]+]] = internal target_declare global i32 0,
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
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* [[VAR_X]], i32* [[VAR_X]], i64 4, i64 35)
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(float* [[VAR_Y]], float* [[VAR_Y]], i64 4, i64 35)
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[VAR_Z]]) ]
#pragma omp target map(x, y)
  {
    x = 5;
    y = 9.0f;
    z = 8;
  }
}
// end INTEL_COLLAB
