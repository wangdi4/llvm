// INTEL_COLLAB
// REQUIRES: clang-target-64-bits
// RUN: %clang_cc1 -opaque-pointers -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fopenmp-late-outline  -o %t.bc %s
// RUN: %clang_cc1 -opaque-pointers -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fopenmp-late-outline -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x]] = internal target_declare global i32 0,
// CHECK: @._ZL1x_[[HASH:[a-f0-9]+]].ref = internal constant ptr @x
#pragma omp declare target
static int x;

void foo() {
// CHECK: store i32 800, ptr [[VAR_X]],
   x = 800;
}
#pragma omp end declare target

// CHECK: define dso_local void [[BAR:@bar]]
void bar() {
// CHECK: [[REGION0:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]])
// CHECK: call void @foo()
// CHECK: call void @llvm.directive.region.exit(token [[REGION0]]) [ "DIR.OMP.END.TARGET"() ]

#pragma omp target map(x)
  {
    foo();
  }

  return;
}

// CHECK: !{{[0-9]+}} = !{i32 {{[0-9]}}, !"_ZL1x_[[HASH]]", i32 0, i32 0, ptr @x}
// end INTEL_COLLAB
