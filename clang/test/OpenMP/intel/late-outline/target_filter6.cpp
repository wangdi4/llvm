// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Verifies that structors declared, used, then defined work in declare
// target situations.

namespace NS1 {
  struct Foo {
    #pragma omp declare target
    Foo();
    #pragma omp end declare target
  };

  #pragma omp declare target
  void func() {
    Foo x;
  }
  #pragma omp end declare target

  Foo::Foo() { }
}

namespace NS2 {
  struct Foo {
    #pragma omp declare target
    ~Foo();
    #pragma omp end declare target
  };

  #pragma omp declare target
  void func() {
    Foo x;
  }
  #pragma omp end declare target

  Foo::~Foo() { }
}

//CHECK: define {{.*}}NS14funcEv{{.*}}#[[TARGET_DECLARE1:[0-9]+]]
//CHECK: define {{.*}}NS13FooC2Ev{{.*}}#[[TARGET_DECLARE2:[0-9]+]]
//CHECK: define {{.*}}NS24funcEv{{.*}}#[[TARGET_DECLARE1]]
//CHECK: define {{.*}}NS23FooD2Ev{{.*}}#[[TARGET_DECLARE1]]

//CHECK: attributes #[[TARGET_DECLARE1]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
