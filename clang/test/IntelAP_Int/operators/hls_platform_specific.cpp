// Check that platform-dependent sizes are followed for HLS.
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s -check-prefix LIN64
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple i386-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s -check-prefix WIN32

// Linux
// LIN64: %[[L0:[0-9]+]] = load i5, i5* %x5_s, align 1
// LIN64: %[[S0:[a-zA-Z0-9]+]] = sext i5 %[[L0]] to i64
// LIN64: store i64 %[[S0]], i64* %l, align 8

// LIN64: %[[L1:[0-9]+]] = load i5, i5* %x5_u, align 1
// LIN64: %[[U0:[a-zA-Z0-9]+]] = zext i5 %[[L1]] to i64
// LIN64: store i64 %[[U0]], i64* %l, align 8

// Windows 32 bit
// WIN32: %[[L0:[0-9]+]] = load i5, i5* %x5_s, align 1
// WIN32: %[[S0:[a-zA-Z0-9]+]] = sext i5 %[[L0]] to i32
// WIN32: store i32 %[[S0]], i32* %l, align 4

// WIN32: %[[L1:[0-9]+]] = load i5, i5* %x5_u, align 1
// WIN32: %[[U0:[a-zA-Z0-9]+]] = zext i5 %[[L1]] to i32
// WIN32: store i32 %[[U0]], i32* %l, align 4

#include "../ihc_apint.h"

void foo() {
  int5_tt x5_s = 0;
  uint5_tt x5_u = 0;
  long l = 0;

  // signed (only those that differ from unsigned)
  l = (long)x5_s;       // ver with unsigned base type has same IR

  // unsigned
  l = (long)x5_u;       // ver with unsigned base type has same IR
}
