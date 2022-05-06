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

struct A {
  #pragma omp declare target
  A();
  #pragma omp end declare target
};

void fn1() {
  A e;

  // Outside target region, these should not generate routines in the
  // device compile.
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME:"QUAL.OMP.PRIVATE:NONPOD"
  //CHECK-SAME: null{{.*}}null
  //CHECK: "DIR.OMP.END.PARALLEL"()
  #pragma omp parallel private(e)
  for(int d=0; d<100; d++);

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME:"QUAL.OMP.PRIVATE:NONPOD"
  //CHECK-SAME: _ZTS1A.omp.def_constr{{.*}}_ZTS1A.omp.destr
  //CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target private(e)
  for(int d=0; d<100; d++);

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME:"QUAL.OMP.FIRSTPRIVATE:NONPOD"
  //CHECK-SAME: _ZTS1A.omp.copy_constr{{.*}}_ZTS1A.omp.destr
  //CHECK: "DIR.OMP.END.TARGET"()
  #pragma omp target firstprivate(e)
  for(int d=0; d<100; d++);
}

// CHECK: define internal{{.*}}.omp.def_constr{{.*}}#[[DECLARE_TARGET:[0-9]+]]
// CHECK: define internal{{.*}}.omp.destr{{.*}} #[[DECLARE_TARGET]]
// CHECK: define internal{{.*}}.omp.copy_constr{{.*}}#[[DECLARE_TARGET]]


// CHECK: attributes #[[DECLARE_TARGET]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
