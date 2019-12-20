// Ensure we can declare a constexpr ap_int.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s

typedef int int65_tt __attribute__((__ap_int(65)));
constexpr int65_tt an_int = 17;

using ap_uint = unsigned int __attribute__((__ap_int(77)));
using ap_int = int __attribute__((__ap_int(77)));

#ifndef SYCL_EXTERNAL
#define SYCL_EXTERNAL
#endif

SYCL_EXTERNAL
int65_tt foo() {
  return an_int;
  //CHECK: ret i65 17
}


constexpr ap_uint u1 = 15;
constexpr ap_uint u2 = 4;
constexpr ap_uint u3 = u1+u2;
constexpr ap_uint u4 = u1-u2;
constexpr ap_uint u5 = u1/u2;
constexpr ap_uint u6 = u1*u2;
constexpr ap_uint u7 = u1%u2;
constexpr ap_uint u8 = u1|u2;
constexpr ap_uint u9 = u1&u2;
constexpr ap_uint u0 = u1^u2;
constexpr ap_uint ua = ~u1;

constexpr ap_int s1 = 15;
constexpr ap_int s2 = 4;
constexpr ap_int s3 = s1+s2;
constexpr ap_int s4 = s1-s2;
constexpr ap_int s5 = s1/s2;
constexpr ap_int s6 = s1*s2;
constexpr ap_int s7 = s1%s2;
constexpr ap_int s8 = s1|s2;
constexpr ap_int s9 = s1&s2;
constexpr ap_int s0 = s1^s2;
constexpr ap_int sa = ~s1;

SYCL_EXTERNAL
ap_uint unsigned_usages(int i) {
  switch (i) {
  case 1:
    return u1;
    // CHECK: store i77 15
  case 2:
    return u2;
    // CHECK: store i77 4
  case 3:
    return u3;
    // CHECK: store i77 19
  case 4:
    return u4;
    // CHECK: store i77 11
  case 5:
    return u5;
    // CHECK: store i77 3
  case 6:
    return u6;
    // CHECK: store i77 60
  case 7:
    return u7;
    // CHECK: store i77 3
  case 8:
    return u8;
    // CHECK: store i77 15
  case 9:
    return u9;
    // CHECK: store i77 4
  case 0:
    return u0;
    // CHECK: store i77 11
  case 10:
    return ua;
    // CHECK: store i77 -16
  }
  return 0;
}

SYCL_EXTERNAL
ap_int signed_usages(int i) {
  switch (i) {
  case 1:
    return s1;
    // CHECK: store i77 15
  case 2:
    return s2;
    // CHECK: store i77 4
  case 3:
    return s3;
    // CHECK: store i77 19
  case 4:
    return s4;
    // CHECK: store i77 11
  case 5:
    return s5;
    // CHECK: store i77 3
  case 6:
    return s6;
    // CHECK: store i77 60
  case 7:
    return s7;
    // CHECK: store i77 3
  case 8:
    return s8;
    // CHECK: store i77 15
  case 9:
    return s9;
    // CHECK: store i77 4
  case 0:
    return s0;
    // CHECK: store i77 11
  case 10:
    return sa;
    // CHECK: store i77 -16
  }
  return 0;
}
