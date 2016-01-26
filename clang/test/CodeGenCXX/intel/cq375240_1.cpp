// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple=x86_64-windows -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

typedef void(FFF)(int i=17);
FFF *F;

void foo(bool b) {
  F(26);
// CHECK: [[F:%.+]] = load void (i32)*, void (i32)** {{@.*F.*}},
// CHECK: call void [[F]](i32 26)
}

