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
// RUN:  | FileCheck %s -check-prefix CHECKCONT
//
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases -DWITH \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases -DWITH \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s -check-prefix CHECKTD
//
// expected-no-diagnostics

// Verifies that the static member appears in the target IR both with and
// without the 'declare target' pragmas.

struct Foo {
#ifdef WITH
#pragma omp declare target
#endif
  static void static_member() {
    #pragma omp target
    {
      int i = 1;
    }
  }
#ifdef WITH
#pragma omp end declare target
#endif
};

void foo()
{
  Foo::static_member();
}

//CHECKTD: define {{.*}}static_member{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//CHECKTD: attributes #[[TARGET_DECLARE]] = {{.*}}"openmp-target-declare"="true"
//CHECKCONT: define {{.*}}static_member{{.*}}#[[CONTAINS:[0-9]+]]
//CHECKCONT: attributes #[[CONTAINS]] = {{.*}}"contains-openmp-target"="true"

// end INTEL_COLLAB
