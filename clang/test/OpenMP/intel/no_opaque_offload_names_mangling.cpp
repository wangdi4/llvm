//RUN: %clang_cc1 -no-opaque-pointers -triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fopenmp-late-outline \
//RUN: -o %t_host.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -target-cpu \
//RUN: x86-64 -fopenmp-late-outline \
//RUN: -o - %s \
//RUN: | FileCheck %s --check-prefix=CHECK-HOST

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc -emit-llvm-bc \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fopenmp-late-outline \
//RUN: -o %t_targ.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 -aux-triple \
//RUN: x86_64-pc-windows-msvc \
//RUN: -emit-llvm -disable-llvm-passes \
//RUN: -fintel-compatibility -fms-compatibility \
//RUN: -fms-extensions -fms-compatibility \
//RUN: -fopenmp -fopenmp-targets=spir64 -fopenmp-is-device \
//RUN: -fopenmp-host-ir-file-path %t_host.bc \
//RUN: -fopenmp-late-outline \
//RUN: -o - %s \
//RUN: | FileCheck %s --check-prefix=CHECK-TARG

//expected-no-diagnostics

#pragma omp declare target
class C1 {
public:
  C1() {
#pragma omp target
    ;
  }
  ~C1() {
#pragma omp target
    ;
  }
};

C1 c1;

class C2 {
public:
  C2() {
  }
  ~C2() {
  }
};

C2 c2;
#pragma omp end declare target

void foo() {
#pragma omp target
  ;
}

// CHECK-HOST-DAG: define{{.*}}%class.C1* @"??0C1@@QEAA@XZ"
// CHECK-HOST-DAG: define{{.*}}void @"??1C1@@QEAA@XZ"
// CHECK-HOST-DAG: define{{.*}}%class.C2* @"??0C2@@QEAA@XZ"
// CHECK-HOST-DAG: define{{.*}}void @"??1C2@@QEAA@XZ"
// CHECK-HOST-DAG: define{{.*}}void @"?foo@@YAXXZ"

// CHECK-HOST: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_ctor"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_dtor"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_ctor"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_dtor"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z3foov"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"_ZN2C1C1Ev"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"_ZN2C1D1Ev"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z2c2",{{.*}}%class.C2* @"?c2@@3VC2@@A"
// CHECK-HOST-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z2c1",{{.*}}%class.C1* @"?c1@@3VC1@@A"

// CHECK-TARG-DAG: define{{.*}}void @__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_ctor
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C1C1Ev
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C1C2Ev
// CHECK-TARG-DAG: define{{.*}}void @__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_dtor
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C1D1Ev
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C1D2Ev
// CHECK-TARG-DAG: define{{.*}}void @__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_ctor
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C2C1Ev
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C2C2Ev
// CHECK-TARG-DAG: define{{.*}}void @__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_dtor
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C2D1Ev
// CHECK-TARG-DAG: define{{.*}}void @_ZN2C2D2Ev
// CHECK-TARG-DAG: define{{.*}}void @_Z3foov

// CHECK-TARG: !omp_offload.info = !{!{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}, !{{[0-9]+}}}
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_ctor"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c1_l53_dtor"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_ctor"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"__omp_offloading_{{[a-f0-9]+}}_{{[a-f0-9]+}}_c2_l63_dtor"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z3foov"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"_ZN2C1C1Ev"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"_ZN2C1D1Ev"
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z2c2",{{.*}}%class.C2 addrspace(1)* @c2
// CHECK-TARG-DAG: !{{[0-9]+}} = !{{{.*}}, !"_Z2c1",{{.*}}%class.C1 addrspace(1)* @c1
