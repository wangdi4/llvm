// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -opaque-pointers -o - | FileCheck %s -check-prefixes=CHECK,SPIR
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -opaque-pointers -o - | FileCheck %s -check-prefixes=CHECK,X86

// SPIR: define{{.*}}spir_kernel void @foo(ptr addrspace(3) nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS:[0-9]+]]
// X86: define{{.*}}spir_kernel void @foo(ptr nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS:[0-9]+]]
__kernel void foo(__local __attribute__((local_mem_size(32))) int *ip) {}

// SPIR: define{{.*}}spir_kernel void @foo2(ptr addrspace(3) nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS64:[0-9]+]]
// X86: define{{.*}}spir_kernel void @foo2(ptr nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS64:[0-9]+]]
__kernel void foo2(__local __attribute__((local_mem_size(64))) int ip[]) {}

// SPIR: define{{.*}}spir_kernel void @foo3(ptr addrspace(3) nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS128:[0-9]+]]
// X86: define{{.*}}spir_kernel void @foo3(ptr nocapture noundef align 4 %ip) local_unnamed_addr #0 !local_mem_size ![[LMS128:[0-9]+]]
__kernel void foo3(__local __attribute__((local_mem_size(128))) int ip[5]) {}

// CHECK: ![[LMS]] = !{i32 32}
// CHECK: ![[LMS64]] = !{i32 64}
// CHECK: ![[LMS128]] = !{i32 128}
