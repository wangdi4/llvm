// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

// CHECK: define spir_func void @foo(i32 addrspace(3)* nocapture %ip) local_unnamed_addr #0 !local_mem_size ![[LMS:[0-9]+]]
void foo(__local __attribute__((local_mem_size(32))) int *ip) {}

// CHECK: ![[LMS]] = !{i32 32}

