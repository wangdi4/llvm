// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

#define QDR \
    __global __attribute__((buffer_location("QDR")))

kernel void t1(global __attribute__((buffer_location("DDR"))) int *a,
               QDR float *b,
               global int *c) {}

kernel void t2(global __attribute__((buffer_location("DDR"))) int a[],
               QDR float b[5],
               global int *c) {}

// CHECK: define spir_kernel void @t1{{[^!]+}}
// CHECK: !kernel_arg_buffer_location ![[MD:[0-9]+]]
// CHECK: define spir_kernel void @t2{{[^!]+}}
// CHECK: !kernel_arg_buffer_location ![[MD]]
// CHECK: ![[MD]] = !{!"DDR", !"QDR", !""}
