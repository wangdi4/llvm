// RUN: %clang_cc1 -triple i386-unknown-unknown -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -emit-llvm -o - %s | FileCheck %s

void __attribute__((regcall)) f1(void);

void f2(void) {
  f1();
// CHECK: call __regcall void @f1()
}

// CHECK: declare __regcall void @f1()
