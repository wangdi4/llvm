//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

void __attribute__((stall_enable)) foo1() {}
// CHECK: @_Z4foo1v{{.*}}!stall_enable [[CFOO1:![0-9]+]]

void foo2() {
  auto lambda = []() __attribute__((stall_enable)){};
  lambda();
  // CHECK: @"_ZZ4foo2vENK3$_0clEv"(%class.anon* %this){{.*}}!stall_enable [[CFOO1]]
}

//CHECK: [[CFOO1]] = !{i32 1}
