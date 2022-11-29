// INTEL_COLLAB
// Check host code generation.
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm %s -o - | FileCheck %s --check-prefix CHECK-HST --check-prefix CHECK-ALL
//
// Check target code generation. Need to create host IR.
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-pc-linux-gnu -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | FileCheck %s --check-prefix CHECK-TGT --check-prefix CHECK-ALL
//
// expected-no-diagnostics

#pragma omp declare target
// Target declare variable should be retained in both compilations.
// CHECK-ALL: @Var1 ={{ hidden | }}{{.*}}target_declare global i32 5
int Var1 = 5;

// Target declare variable with constructor/destructor. It should be retained in
// both compilations. In addition front end should create constructor/destructor
// entries for the variable.
// CHECK-ALL: @Var2 ={{.*}}target_declare global %struct.S zeroinitializer
// CHECK-HST: @[[CTOR:__omp_offloading.*_ctor]] = private constant
// CHECK-HST: @[[DTOR:__omp_offloading.*_dtor]] = private constant
struct S {
  S() {}
  ~S() {}
};
S Var2;
#pragma omp end declare target

// CHECK-TGT: define{{ hidden | }}{{.*}}void @_Z3goov()
// CHECK-TGT-NEXT: entry:
// CHECK-TGT-NEXT: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]]) ]
// CHECK-TGT-NEXT: call void @_Z3barv()
// CHECK-TGT-NEXT: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]

// CHECK-TGT: define {{.*}}void @[[CTOR:__omp_offloading.*_ctor]]()
// CHECK-TGT: define {{.*}}void @[[DTOR:__omp_offloading.*_dtor]]()

// Target IR shoud not have variables that are not defined as "declare target".
// CHECK-HST:     @Var3 = {{.*}}global i32 10
// CHECK-TGT-NOT: @Var3
int Var3 = 10;

// Host only function. Should not be emitted for the target.
// CHECK-HST:     define{{.*}}void @_Z3foov()
// CHECK-TGT-NOT: define{{.*}}void @_Z3foov()
void foo() {}

// Implict target declare function. Should always be emitted for the host.
// CHECK-HST: define{{.*}}void @_Z3barv()
void bar() {}

// Function with target region.
// CHECK-HST: define{{ hidden | }}{{.*}}void @_Z3goov()
void goo() {
  // CHECK-HST-NEXT: entry:
  // CHECK-HST-NEXT: [[REGION:%[0-9]+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 [[ENTRYIDX:[0-9]+]]) ]
  // CHECK-HST-NEXT: call void @_Z3barv()
  // CHECK-HST-NEXT: call void @llvm.directive.region.exit(token [[REGION]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target
  bar();
}

// Implict target declare function. Should be in target compilation.
// CHECK-TGT: define{{ hidden | }}{{.*}}void @_Z3barv()

// Offload registration code should not be emitted with late outlining.
// CHECK-ALL-NOT: @__tgt_register_lib
// CHECK-ALL-NOT: @__tgt_unregister_lib

// Check that metadata is generated.
// CHECK-ALL: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"[[CTOR]]", i32 {{-?[0-9]+}}, i32 0, i32 {{[0-9]+}}, i32 2}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"[[DTOR]]", i32 {{-?[0-9]+}}, i32 0, i32 {{[0-9]+}}, i32 4}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 0, i32 {{-?[0-9]+}}, i32 {{-?[0-9]+}}, !"_Z3goov", i32 {{-?[0-9]+}}, i32 0, i32 [[ENTRYIDX]], i32 0}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 1, !"_Z4Var1", i32 {{[0-9]+}}, i32 {{[0-9]+}}, i32* @Var1}
// CHECK-ALL-DAG: !{{[0-9]+}} = !{i32 1, !"_Z4Var2", i32 {{[0-9]+}}, i32 {{[0-9]+}}, %struct.S* @Var2}
// end INTEL_COLLAB
