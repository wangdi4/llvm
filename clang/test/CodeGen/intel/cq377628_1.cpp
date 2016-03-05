// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple=x86_64-windows -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

int foo()
{
  extern int iii;
  return iii;
// CHECK: [[t0:%.+]] = load i32, i32* [[iii:@.+]],
// CHECK: ret i32 [[t0]]
}

extern "C" {
int bar()
{
// CHECK: [[t1:%.+]] = load i32, i32* [[iii]],
// CHECK: ret i32 [[t1]]
  extern int iii;
  return iii;
}
}
