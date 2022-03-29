// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Verifies that declare target directives around file scope variables with
// ctors and dtors are handled correctly.  The ctor and dtor routines should
// be external, and referenced functions marked openmp-target-declare.

struct Foo {
  Foo() {}
  ~Foo() {}
};

#pragma omp declare target
Foo A;
#pragma omp end declare target

int main(void)
{
  int i = 3;
  #pragma omp target
  {
    i++;
  }

  return 0;
}

//CHECK: define {{.*}}spir_func void {{.*}}@__omp_offloading_{{.*}}_ctor()
//CHECK: define {{.*}}ZN3FooC2Ev{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//CHECK: define {{.*}}spir_func void {{.*}}@__omp_offloading_{{.*}}_dtor()
//CHECK: define {{.*}}ZN3FooD2Ev{{.*}}#[[TARGET_DECLARE:[0-9]+]]

//CHECK: attributes #[[TARGET_DECLARE]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
