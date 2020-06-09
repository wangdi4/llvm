// INTEL_COLLAB

// Checks that -mlong-double-64 and -mlong-double-128 are matched on
// the host and target compiles.

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-64 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -emit-llvm %s -o - | FileCheck %s -check-prefix CHECK64

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-64 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-64 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -emit-llvm %s -o - | FileCheck %s -check-prefix CHECK64
//
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-128 -DHASFP128 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu \
// RUN:  -emit-llvm %s -o - | FileCheck %s -check-prefix CHECK128

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-128 -DHASFP128 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=x86_64-pc-linux-gnu \
// RUN:  -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu \
// RUN:   -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -mlong-double-128 -DHASFP128 \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc \
// RUN:  -emit-llvm %s -o - | FileCheck %s -check-prefix CHECK128
//
// expected-no-diagnostics

//CHECK64-LABEL: foo
int foo() {

  //CHECK64: [[HOST_VAL:%host_val.*]] = alloca double,
  //CHECK64: [[FP_RES:%fp_res.*]] = alloca double,
  //CHECK64: [[XYZ:%xyz.*]] = alloca double,
  long double host_val = 2.0;
  #pragma omp target
  {
    long double  fp_res;
    const long double xyz = 0.5;
    fp_res = xyz;
  }
  return (int)host_val;
}

#ifdef HASFP128
//CHECK128-LABEL: bar
int bar() {
  //CHECK128: [[HOST_VAL:%host_val.*]] = alloca fp128,
  //CHECK128: [[FP_RES:%fp_res.*]] = alloca fp128,
  //CHECK128: [[XYZ:%xyz.*]] = alloca fp128,
  long double host_val = 2.0;
  #pragma omp target
  {
    long double  fp_res;
    const long double xyz = 0.5;
    fp_res = xyz;
  }
  return (int)host_val;
}
#endif // HASFP128

// end INTEL_COLLAB
