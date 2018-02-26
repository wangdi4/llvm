// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm -o - | FileCheck %s

#define QDR \
    __global __attribute__((buffer_location("QDR")))

kernel void t1(global __attribute__((buffer_location("DDR"))) int *a,
               QDR float *b,
               global int *c) {}

// CHECK: define spir_kernel void @t1{{[^!]+}}
// CHECK: !kernel_arg_buffer_location ![[MD:[0-9]+]]
// CHECK: ![[MD]] = !{!"DDR", !"QDR", !""}
