// INTEL_COLLAB
// Check target code generation. Need to create host IR.
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | FileCheck %s
//
// expected-no-diagnostics

// Static function with target region.
// CHECK: define internal void @_ZL3foov()
static void foo() {
// CHECK-NEXT: entry:
// CHECK-NEXT: [[REGION0:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[IDX0:[0-9]+]]) ]
// CHECK-NEXT: call void @llvm.directive.region.exit(token [[REGION0]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  {}
}

// Inline function with target region.
// CHECK: define linkonce_odr{{ hidden | }}void @_Z3barv()
inline void bar() {
// CHECK-NEXT: entry:
// CHECK-NEXT: [[REGION1:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[IDX1:[0-9]+]]) ]
// CHECK-NEXT: call void @llvm.directive.region.exit(token [[REGION1]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  {}
}

struct S {
  void func();
};

void S::func() {
  foo();
  bar();
}

// Check that metadata is generated.
// CHECK: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"_ZL3foov", i32 {{[0-9]+}}, i32 0, i32 [[IDX0]], i32 0}
// CHECK-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"_Z3barv", i32 {{[0-9]+}}, i32 0, i32 [[IDX1]], i32 0}
// end INTEL_COLLAB
