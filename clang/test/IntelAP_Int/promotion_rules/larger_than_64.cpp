// Ensure that operands are allowed to exceed 64 bits for HLS

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s
// CHECK: %[[L0:[0-9]+]] = load i65, i65* %x65_s, align 8
// CHECK: %[[L1:[0-9]+]] = load i32, i32* %u_int_var, align 4
// CHECK: %[[M0:[a-zA-Z0-9]+]] = zext i32 %[[L1]] to i65
// CHECK: %[[M1:[a-zA-Z0-9]+]] = add nsw i65 %[[L0]], %[[M0]]
// CHECK: %[[M2:[a-zA-Z0-9]+]] = trunc i65 %[[M1]] to i64
// CHECK: store i64 %[[M2]], i64* %res, align 8

typedef int int65_tt __attribute__((__ap_int(65)));

void foo() {
  int65_tt x65_s = 0;

  unsigned int u_int_var = 0;

  long long res = 0;

  res = x65_s + u_int_var;
}
