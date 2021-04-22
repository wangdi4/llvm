// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL2.0 -emit-llvm -o - | FileCheck %s
#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : enable

kernel void k1(read_only pipe int p1 __attribute__((intel_host_accessible)),
               write_only pipe float p2 __attribute__((intel_host_accessible)),
               write_only pipe float p3,
               int i,
               __global int* pi) {
}

// CHECK: define{{.*}}spir_kernel void @k1{{[^!]+}}
// CHECK: !kernel_arg_host_accessible ![[MD:[0-9]+]]
// CHECK: ![[MD]] = !{i1 true, i1 true, i1 false, i1 false, i1 false}
