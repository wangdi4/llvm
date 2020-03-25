// RUN: %clang_cc1 %s -mx87-precision=32 -emit-llvm -o - | FileCheck %s --check-prefixes=PC32
// RUN: %clang_cc1 %s -mx87-precision=64 -emit-llvm -o - | FileCheck %s --check-prefixes=PC64
// RUN: %clang_cc1 %s -mx87-precision=80 -emit-llvm -o - | FileCheck %s --check-prefixes=PC80

int main() {
  // PC32: "x87-precision"="32"
  // PC64: "x87-precision"="64"
  // PC80: "x87-precision"="80"
  return 0;
}
