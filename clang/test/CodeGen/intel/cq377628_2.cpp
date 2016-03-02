// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple=x86_64-windows -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

extern "C" {
int bar()
{
  extern int iii;
  return iii;
// CHECK: [[t0:%.+]] = load i32, i32* [[iii:@i.+]],
// CHECK: ret i32 [[t0]]
}
}

extern "C++" {
int foo()
{
  extern int iii;
  return iii;
// CHECK: [[t1:%.+]] = load i32, i32* [[iii]],
// CHECK: ret i32 [[t1]]
}
}
