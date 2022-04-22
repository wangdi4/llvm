// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN: -mconstructor-aliases \
// RUN: -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

struct MyTOne { double val; MyTOne() {val = 0.0; } };
#pragma omp declare reduction(+: MyTOne : omp_out.val += omp_in.val)

struct MyTTwo { double val; MyTTwo() {val = 0.0; } };
#pragma omp declare reduction(+: MyTTwo : omp_out.val += omp_in.val)

struct MyTThree { double val; MyTThree() {val = 0.0; } };
#pragma omp declare reduction(+: MyTThree : omp_out.val += omp_in.val)

struct MyTFour { double val; MyTFour() {val = 0.0; } };
#pragma omp declare reduction(+: MyTFour : omp_out.val += omp_in.val)

struct MyTFive { double val; MyTFive() {val = 0.0; } };
#pragma omp declare reduction(+: MyTFive : omp_out.val += omp_in.val)

// CHECK: define {{.*}}foo{{.*}}#[[CONTAINS:[0-9]+]]

// CHECK: define {{.*}}omp_combiner{{.*}}MyTOne{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTOne{{.*}}omp.def_constr{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTOne.omp.destr{{.*}}#[[DECL_TARGET:[0-9]+]]

// CHECK: define {{.*}}omp_combiner{{.*}}MyTTwo{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTTwo{{.*}}omp.def_constr{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTTwo.omp.destr{{.*}}#[[DECL_TARGET:[0-9]+]]

// CHECK: define {{.*}}omp_combiner{{.*}}MyTThree{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTThree{{.*}}omp.def_constr{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTThree.omp.destr{{.*}}#[[DECL_TARGET:[0-9]+]]

// CHECK: define {{.*}}omp_combiner{{.*}}MyTFour{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTFour{{.*}}omp.def_constr{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTFour.omp.destr{{.*}}#[[DECL_TARGET:[0-9]+]]

// CHECK: define {{.*}}omp_combiner{{.*}}MyTFive{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTFive{{.*}}omp.def_constr{{.*}}#[[DECL_TARGET:[0-9]+]]
// CHECK: define {{.*}}MyTFive.omp.destr{{.*}}#[[DECL_TARGET:[0-9]+]]

void foo()
{
  MyTOne my1;
  #pragma omp target parallel reduction(+:my1)
  {
    my1.val = 1.0;
  }

  MyTTwo my2;
  #pragma omp target parallel for reduction(+:my2)
  for (int i=0;i<8;++i)
  {
    my2.val = 1.0+i;
  }

  MyTThree my3;
  #pragma omp target teams reduction(+:my3)
  #pragma omp parallel
  {
    my3.val = 1.0;
  }

  MyTFour my4;
  #pragma omp target teams reduction(+:my4)
  #pragma omp distribute
  for (int i=0;i<8;++i)
  {
    my4.val = 1.0+i;
  }

  MyTFive my5;
  #pragma omp target teams distribute reduction(+:my5)
  for (int i=0;i<8;++i)
  {
    my5.val = 1.0+i;
  }
}

// CHECK: attributes #[[CONTAINS]] = {{.*}}"contains-openmp-target"="true"
// CHECK: attributes #[[DECL_TARGET]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
