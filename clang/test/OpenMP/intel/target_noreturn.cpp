// RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -std=c++14 -triple x86_64-unknown-linux-gnu \
// RUN:   -emit-llvm-bc -o %t_host.bc %s

// RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fopenmp-targets=spir64 -std=c++14 -triple spir64 \
// RUN:   -fopenmp-host-ir-file-path %t_host.bc -fopenmp-is-device \
// RUN:   -emit-llvm -o - %s | FileCheck %s

extern "C" void assert_fail (void) __attribute__ ((__noreturn__));

void foo(int i)
{
  #pragma omp target
  {
    if (i) {
      assert_fail();
//CHECK-NOT: unreachable
    }
  }
}
