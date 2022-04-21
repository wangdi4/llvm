// INTEL_COLLAB
// REQUIRES: clang-target-64-bits
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fopenmp-late-outline  -o %t.bc %s
// RUN: %clang_cc1 -no-opaque-pointers -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fopenmp-late-outline -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_I:@i]] = external {{.*}}global i32,
static int i;

void foo() {
// CHECK: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]]),
// CHECK-SAME "QUAL.OMP.FIRSTPRIVATE"(i32* [[VAR_I]]) ]
// CHECK: store i32 3, i32* [[VAR_I]]
// CHECK: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]

#pragma omp target
  i = 3;
}

// CHECK-NOT: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"{{[a-zA-Z_0-9]+}}i_{{[a-f0-9]+}}"{{.*}}, i32* @i}
// end INTEL_COLLAB
