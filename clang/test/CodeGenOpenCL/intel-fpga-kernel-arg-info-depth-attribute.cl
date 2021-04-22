// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm %s -o - | FileCheck %s

kernel void k1(read_only pipe int p1 __attribute__((depth(10))),
               write_only pipe float p2 __attribute__((depth(25))),
               write_only pipe float p3,
               int i,
               __global int* pi) {
}

// CHECK: define{{.*}}spir_kernel void @k1{{[^!]+}}
// CHECK: !kernel_arg_pipe_depth ![[MD:[0-9]+]]
// CHECK: ![[MD]] = !{i32 10, i32 25, i32 0, i32 0, i32 0}
