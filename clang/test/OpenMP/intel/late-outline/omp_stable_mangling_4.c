// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fintel-openmp-region  -o %t.bc %s
// RUN: %clang_cc1 -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fintel-openmp-region -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_I:@i_[0-9a-f]+]] = external {{.*}}global i32,
static int i;

void foo() {
// CHECK: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]]),
// CHECK-SAME "QUAL.OMP.FIRSTPRIVATE"(i32* [[VAR_I]]) ]
// CHECK: store i32 3, i32* [[VAR_I]]
// CHECK: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]

#pragma omp target
  i = 3;
}
// end INTEL_COLLAB
