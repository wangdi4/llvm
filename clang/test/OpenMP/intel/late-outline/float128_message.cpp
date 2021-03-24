// INTEL_COLLAB
// Test target codegen - host bc file has to be created first.
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -aux-triple x86_64-unknown-linux-gnu -fintel-compatibility \
// RUN:  -fopenmp-late-outline  -fopenmp-targets=spir64 %s \
// RUN: -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc -fsyntax-only

#define floatDef __float128
// expected-note@+1 {{'f' defined here}}
floatDef foo(floatDef f) {
#pragma omp target map(f)
  // expected-error@+1 {{'f' requires 128 bit size '__float128' type support, but device 'spir64' does not support it}}
  f = 1.0000;
  return f;
}

// expected-note@+1 {{'d' defined here}}
long double zoo(long double d)
{
#pragma omp target map(d)
  // expected-error@+1 {{'d' requires 128 bit size 'long double' type support, but device 'spir64' does not support it}}
  d = 1.0;
  return d;
}
// end INTEL_COLLAB
