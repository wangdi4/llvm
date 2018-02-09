// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s --check-prefix CHECK32
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s --check-prefix CHECK64

__attribute__((max_global_work_dim(0)))
__attribute__((num_compute_units(2, 2)))
__attribute__((autorun))
__kernel void foo() {
  int x = get_compute_id(0);
// CHECK32: call i32 @get_compute_id(i32 0)
// CHECK64: call i64 @get_compute_id(i32 0)
}

// CHECK32: declare i32 @get_compute_id(i32)
// CHECK64: declare i64 @get_compute_id(i32)
