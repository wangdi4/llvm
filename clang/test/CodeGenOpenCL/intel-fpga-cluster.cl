// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

void __attribute__((cluster)) foo1() {}
// CHECK: @foo1{{.*}}!cluster [[CFOO1:![0-9]+]]

void __attribute__((cluster("clustername"))) foo2() {}
// CHECK: @foo2{{.*}}!cluster [[CFOO2:![0-9]+]]

void __attribute__((cluster(""))) foo3() {}
// CHECK: @foo3{{.*}}!cluster [[CFOO3:![0-9]+]]

//CHECK: [[CFOO1]] = !{!"", i32 0}
//CHECK: [[CFOO2]] = !{!"clustername", i32 1}
//CHECK: [[CFOO3]] = !{!"", i32 1}
