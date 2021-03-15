// Ensure successful parsing of ac_int.h

// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -emit-llvm -o %t
// RUN: FileCheck %s < %t

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device %s -emit-llvm -o %t
// RUN: FileCheck %s < %t

// CHECK: = add nsw i66

#include "ac_int.h"

#ifdef SYCL_EXTERNAL
SYCL_EXTERNAL
#endif
void foo() {
  ac_int<65, true> x65_s = 0, y65_s = 0;
  unsigned int u_int_var = 0;

  y65_s = x65_s + u_int_var;
}
