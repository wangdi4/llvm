// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,SPIR
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,X86

// SPIR: define spir_kernel void @foo(i32 addrspace(3)* nocapture %ip) local_unnamed_addr #0 !local_mem_size ![[LMS:[0-9]+]]
// X86: define spir_kernel void @foo(i32* nocapture %ip) local_unnamed_addr #0 !local_mem_size ![[LMS:[0-9]+]]
__kernel void foo(__local __attribute__((local_mem_size(32))) int *ip) {}

// CHECK: ![[LMS]] = !{i32 32}
