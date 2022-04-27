//RUN: %clang_cc1 -fhls -emit-llvm -no-opaque-pointers -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

void __attribute__((use_stall_enable_clusters)) foo1() {}
// CHECK: @_Z4foo1v{{.*}}!stall_enable [[CFOO1:![0-9]+]]

void foo2() {
  auto lambda = []() __attribute__((use_stall_enable_clusters)){};
  lambda();
  // CHECK: @"_ZZ4foo2vENK3$_0clEv"(%class.anon* {{[^,]*}} %this){{.*}}!stall_enable [[CFOO1]]
}

//CHECK: [[CFOO1]] = !{i32 1}
