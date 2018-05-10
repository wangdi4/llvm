// Ensure we can declare a constexpr ap_int.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s

typedef int int65_tt __attribute__((__ap_int(65)));
constexpr int65_tt an_int = 17;

int65_tt foo() {
  return an_int;
  //CHECK: ret i65 17
}
