// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

void __attribute__((use_stall_enable_clusters)) foo1() {}
// CHECK: @foo1{{.*}}!stall_enable [[CFOO1:![0-9]+]]

//CHECK: [[CFOO1]] = !{i32 1}
