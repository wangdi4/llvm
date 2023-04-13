// INTEL_COLLAB

// This case happens when user specifies -mlong-double-64. Host and target
// 'long double' match and the code is allowed.

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -mlong-double-64 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -std=c++17 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 -std=c++17 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -mlong-double-64 -emit-llvm %s -o %t-targ.ll

// This is the normal case. User does not specify 'long double' size. The host
// and target do not match and an error is given on use.

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -std=c++17 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 -std=c++17 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -DERR -emit-llvm %s -o %t-targ.ll

void foo()
{
#ifndef ERR
//expected-no-diagnostics
#else
  //expected-error@+6 {{'x' requires 128 bit size 'long double' type support, but target 'spir64' does not support it}}
  //expected-note@+2 {{'x' defined here}}
#endif
  long double x = 0.0;
  #pragma omp target map(tofrom:x)
  {
    x = 1.259921049894873164767210607278228350570L;
  }
}
// end INTEL_COLLAB
