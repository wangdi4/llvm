// INTEL_COLLAB
// REQUIRES: clang-target-64-bits
// RUN: %clang_cc1 -opaque-pointers -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fopenmp-late-outline  -o %t.bc %s
// RUN: %clang_cc1 -opaque-pointers -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fopenmp-late-outline -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_I:@i]] = external {{.*}}global i32,
static int i;

void bar() {
  i = 9;
}

void foo() {
// CHECK: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry()
// CHECK-SAME: "DIR.OMP.TARGET"()
// CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]])
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[VAR_I]], ptr [[VAR_I]], i64 4, i64 35
// CHECK: store i32 3, ptr [[VAR_I]],
// CHECK: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]

#pragma omp target map(tofrom:i)
  i = 3;
}

// CHECK-NOT: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"{{[a-zA-Z_0-9]+}}i_{{[a-f0-9]+}}"{{.*}}, ptr @i}
// end INTEL_COLLAB
