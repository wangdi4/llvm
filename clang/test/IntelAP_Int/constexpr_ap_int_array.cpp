// Ensure we can declare constexpr arrays of ap_ints.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s

typedef int int65_tt __attribute__((__ap_int(65)));
constexpr int65_tt an_array[] = {1,2,3};
// CHECK: @[[ARRAY:[a-zA-Za-zA-Z0-9_]+]] = internal constant [3 x i65] [i65 1, i65 2, i65 3], align 16

int65_tt foo(int idx) {
  return an_array[idx];
}
