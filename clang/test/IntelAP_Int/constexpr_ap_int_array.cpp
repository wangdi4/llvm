// Ensure we can declare constexpr arrays of ap_ints.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=HLS

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-windows-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple spir64-unknown-linux-sycldevice -fsycl-is-device -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=SYCL

typedef int int65_tt __attribute__((__ap_int(65)));
constexpr int65_tt an_array[] = {1,2,3};
// HLS: @[[ARRAY:[a-zA-Za-zA-Z0-9_]+]] = internal constant [3 x i65] [i65 1, i65 2, i65 3], align 16
// SYCL: @[[ARRAY:[a-zA-Za-zA-Z0-9_]+]] = internal addrspace(1) constant [3 x i65] [i65 1, i65 2, i65 3], align 8

#ifdef SYCL_EXTERNAL
SYCL_EXTERNAL
#endif
int65_tt foo(int idx) {
  return an_array[idx];
}
