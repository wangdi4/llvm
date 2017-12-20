// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

__attribute__((max_work_group_size(1024, 1, 1)))
__kernel void k1() {}
// CHECK: define spir_kernel void @k1{{[^{]+}} !max_work_group_size ![[MD1:[0-9]+]]

__attribute__((task))
__kernel void k2() {}
// CHECK: define spir_kernel void @k2{{[^{]+}} !task ![[MD2:[0-9]+]]

__attribute__((reqd_work_group_size(16,16,16)))
__kernel void k3() {}
// CHECK: define spir_kernel void @k3{{[^{]+}} !reqd_work_group_size ![[MD3:[0-9]+]]

__attribute__((max_work_group_size(16,16,16)))
__kernel void k4() {}
// CHECK: define spir_kernel void @k4{{[^{]+}} !max_work_group_size ![[MD4:[0-9]+]]

__attribute__((reqd_work_group_size(64,64,64)))
__kernel void k5() {}
// CHECK: define spir_kernel void @k5{{[^{]+}} !reqd_work_group_size ![[MD5:[0-9]+]]

__kernel void k6() __attribute__((num_compute_units(3))) {}
// CHECK: define spir_kernel void @k6{{[^{]+}} !num_compute_units ![[MD6:[0-9]+]]

__kernel void k7() __attribute__((num_simd_work_items(4))) {}
// CHECK: define spir_kernel void @k7{{[^{]+}} !num_simd_work_items ![[MD7:[0-9]+]]

__kernel void k8() __attribute__((num_compute_units(3, 2, 4))) {}
// CHECK: define spir_kernel void @k8{{[^{]+}} !num_compute_units ![[MD8:[0-9]+]]

__kernel void k9() __attribute__((max_global_work_dim(0))) {}
// CHECK: define spir_kernel void @k9{{[^{]+}} !task ![[MD2:[0-9]+]]

__kernel void k10() __attribute__((max_global_work_dim(0))) __attribute__((autorun)) {}
// CHECK: define spir_kernel void @k10{{[^{]+}} !task ![[MD2]] !autorun ![[MD2]]

// CHECK-DAG: [[MD1]] = !{i32 1024, i32 1, i32 1}
// CHECK-DAG: [[MD2]] = !{i1 true}
// CHECK-DAG: [[MD3]] = !{i32 16, i32 16, i32 16}
// CHECK-DAG: [[MD4]] = !{i32 16, i32 16, i32 16}
// CHECK-DAG: [[MD5]] = !{i32 64, i32 64, i32 64}
// CHECK-DAG: [[MD6]] = !{i32 3, i32 1, i32 1}
// CHECK-DAG: [[MD7]] = !{i32 4}
// CHECK-DAG: [[MD8]] = !{i32 3, i32 2, i32 4}
