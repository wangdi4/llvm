// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s -fexceptions -std=gnu++11 -fintel-compatibility | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-win32 -emit-llvm -o - %s -fexceptions -std=c++11 -fintel-compatibility -fintel-ms-compatibility | FileCheck %s
// CHECK-LABEL: foo
void foo(const void* handler) {
  // CHECK: bitcast i8* %{{.+}} to void (i32)*
   reinterpret_cast<void (*)(int)>(handler);
}

