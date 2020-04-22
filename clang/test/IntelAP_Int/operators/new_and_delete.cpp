// Ensure that new and delete operators are allowed for ap_ints in the testbench
// Note: the function names below are triple-dependent. These operations must also work with Micrsoft mangling
// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -triple x86_64-linux-pc -emit-llvm -o - | FileCheck %s -check-prefix LIN
// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -triple x86_64-windows-pc -emit-llvm -o - | FileCheck %s -check-prefix WIN
// LIN: = call noalias nonnull i8* @_Znwm(i64 8)
// LIN: call void @_ZdlPv(i8* %{{[0-9]+}})

// WIN: = call noalias nonnull i8* @"??2@YAPEAX_K@Z"(i64 8)
// WIN: call void @"??3@YAXPEAX@Z"(i8* %{{[0-9]+}})

#include "../ihc_apint.h"

void foo() {
  ap_uint<37> *p = new ap_uint<37>;
  delete p;
}
