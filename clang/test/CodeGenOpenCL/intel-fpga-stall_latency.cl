// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm \
// RUN:  -o - %s | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm \
// RUN:  -o - %s | FileCheck %s

__kernel __attribute((stall_latency_enabled))
void kfoo1() {}
//CHECK: define{{.*}}kfoo1{{.*}}!stall_latency [[TRUE:![0-9]+]]

__kernel __attribute((stall_latency_disabled))
void kfoo2() {}
//CHECK: define{{.*}}kfoo2{{.*}}!stall_latency [[FALSE:![0-9]+]]

//CHECK: [[TRUE]] = !{i32 1}
//CHECK: [[FALSE]] = !{i32 0}
