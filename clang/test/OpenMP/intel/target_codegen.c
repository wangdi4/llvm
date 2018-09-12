// Check host code generation.
// RUN: %clang_cc1 -verify -x c -triple x86_64-unknown-linux-gnu -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix CHECK-HST --check-prefix CHECK-ALL
//
// Check target code generation. Need to create host IR.
// RUN: %clang_cc1 -verify -x c -triple x86_64-unknown-linux-gnu -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
// RUN: %clang_cc1 -verify -x c -triple x86_64-pc-linux-gnu -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | FileCheck %s --check-prefix CHECK-TGT --check-prefix CHECK-ALL
//
// expected-no-diagnostics

// Target declare variable should be retained in both compilations.
// CHECK-ALL: @Var1 = global i32 5
#pragma omp declare target
int Var1 = 5;
#pragma omp end declare target

// Target IR shoud not have variables that are not defined as "declare target".
// CHECK-HST:     @Var2 = global i32 10
// CHECK-TGT-NOT: @Var2
int Var2 = 10;

// Host only function. Should not be emitted for the target.
// CHECK-HST:     define void @foo()
// CHECK-TGT-NOT: define void @foo()
void foo() {}

// Implict target declare function. Should always be emitted for the host.
// CHECK-HST: define void @bar()
void bar() {}

// Function with target region.
// CHECK-ALL: define void @goo()
void goo() {
  // CHECK-ALL-NEXT: entry:
  // CHECK-ALL-NEXT: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"() ], !omp_offload.entry ![[ENTRYMD:[0-9]+]]
  // CHECK-ALL-NEXT: call void @bar()
  // CHECK-ALL-NEXT: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  bar();
}

// Implict target declare function. Should be in target compilation.
// CHECK-TGT: define void @bar()

// Offload registration code should not be emitted with late outlining.
// CHECK-ALL-NOT: @__tgt_register_lib
// CHECK-ALL-NOT: @__tgt_unregister_lib

// Check that metadata is generated.
// CHECK-ALL: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"goo", i32 37, i32 [[ORDER:[0-9]+]]}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 1, !"Var1", i32 {{[0-9]+}}, i32 {{[0-9]+}}}
// CHECK-ALL-DAG: ![[ENTRYMD]] = distinct !{i32 [[ORDER]]}
