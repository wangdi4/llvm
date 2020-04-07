// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm-bc -main-file-name %s -fintel-compatibility -fopenmp  -fopenmp-targets=spir64 -disable-llvm-passes -fintel-openmp-region  -o %t.bc %s
// RUN: %clang_cc1 -main-file-name %s -fintel-compatibility -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device -fopenmp-host-ir-file-path %t.bc -O0 -fintel-openmp-region -fopenmp-threadprivate-legacy -emit-llvm-bc -x c %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[VAR_X:@x_[0-9a-f]+]] = internal target_declare global i32 0,
#pragma omp declare target
static int x;

void foo() {
// CHECK: store i32 800, i32* [[VAR_X]],
   x = 800;
}
#pragma omp end declare target

// CHECK: define hidden void [[BAR:@bar]]
void bar() {
// CHECK: [[REGION0:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]])
// CHECK: call void @foo()
// CHECK: call void @llvm.directive.region.exit(token [[REGION0]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK: [[REGION1:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"()
// CHECK-SAME "QUAL.OMP.FROMQUAL.OMP.MAP.FROM"(i32* @x_a{{.*}}, i32* @x_a{{.*}}, i64 4, i64 34) ]
// CHECK: call void @llvm.directive.region.exit(token [[REGION1]]) [ "DIR.OMP.END.TARGET.UPDATE"() ]

#pragma omp target map(x)
  {
    foo();
  }

#pragma omp target update from(x)

  return;
}
// end INTEL_COLLAB
