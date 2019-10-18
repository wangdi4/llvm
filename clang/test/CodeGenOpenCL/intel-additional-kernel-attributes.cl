// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

//CHECK: [[IMD:@.str[\.]*[0-9]*]] = {{.*}}{internal_max_block_ram_depth:64}

__attribute__((max_work_group_size(1024, 1, 1)))
__kernel void k1() {}
// CHECK: define spir_kernel void @k1{{[^{]+}} !max_work_group_size ![[MD1:[0-9]+]]

__attribute__((max_global_work_dim(0)))
__kernel void k2() {}
// CHECK: define spir_kernel void @k2{{[^{]+}} !max_global_work_dim ![[MD2:[0-9]+]]

__attribute__((reqd_work_group_size(16,16,16)))
__kernel void k3() {}
// CHECK: define spir_kernel void @k3{{[^{]+}} !reqd_work_group_size ![[MD3:[0-9]+]]

__attribute__((max_work_group_size(16,16,16)))
__kernel void k4() {}
// CHECK: define spir_kernel void @k4{{[^{]+}} !max_work_group_size ![[MD3]]

__attribute__((reqd_work_group_size(64,64,64)))
__kernel void k5() {}
// CHECK: define spir_kernel void @k5{{[^{]+}} !reqd_work_group_size ![[MD5:[0-9]+]]

__kernel void k6() __attribute__((num_compute_units(3))) {}
// CHECK: define spir_kernel void @k6{{[^{]+}} !num_compute_units ![[MD6:[0-9]+]]

__kernel void k7() __attribute__((num_simd_work_items(4))) {}
// CHECK: define spir_kernel void @k7{{[^{]+}} !num_simd_work_items ![[MD7:[0-9]+]]

__kernel void k8() __attribute__((num_compute_units(3, 2, 4))) {}
// CHECK: define spir_kernel void @k8{{[^{]+}} !num_compute_units ![[MD8:[0-9]+]]

__kernel void k9() __attribute__((max_global_work_dim(1))) {}
// CHECK: define spir_kernel void @k9{{[^{]+}} !max_global_work_dim ![[MD9:[0-9]+]]

__kernel void k10() __attribute__((max_global_work_dim(0))) __attribute__((autorun)) {}
// CHECK: define spir_kernel void @k10{{[^{]+}} !max_global_work_dim ![[MD2]] !autorun ![[MD10:[0-9]+]]

__kernel void k11() __attribute__((stall_free)) {}
// CHECK: define spir_kernel void @k11{{[^{]+}} !stall_free ![[MD10]]

__kernel void k12() __attribute__((scheduler_target_fmax_mhz(12))) {}
// CHECK: define spir_kernel void @k12{{[^{]+}} !scheduler_target_fmax_mhz ![[MD12:[0-9]+]]

__kernel void k13() {
// CHECK: define spir_kernel void @k13{{[^{]+}}
// CHECK: stuff = alloca [100 x i32], align
// CHECK: %[[STUFFBC:[0-9]+]] = bitcast [100 x i32]* %stuff to i8*
    int stuff[100] __attribute__((__internal_max_block_ram_depth__(64)));
// CHECK: llvm.var.annotation{{.*}}[[STUFFBC]]{{.*}}[[IMD]]
}

__kernel void k14() __attribute__((uses_global_work_offset(0))) {}
// CHECK: define spir_kernel void @k14{{[^{]+}} !uses_global_work_offset ![[MD13:[0-9]+]]

__kernel void k15() __attribute__((uses_global_work_offset(1))) {}
// CHECK: define spir_kernel void @k15{{[^{]+}} !uses_global_work_offset ![[MD10]]

__kernel void k16() __attribute__((uses_global_work_offset(31))) {}
// CHECK: define spir_kernel void @k16{{[^{]+}} !uses_global_work_offset ![[MD10]]

// CHECK-DAG: [[MD1]] = !{i32 1024, i32 1, i32 1}
// CHECK-DAG: [[MD2]] = !{i32 0}
// CHECK-DAG: [[MD3]] = !{i32 16, i32 16, i32 16}
// CHECK-DAG: [[MD5]] = !{i32 64, i32 64, i32 64}
// CHECK-DAG: [[MD6]] = !{i32 3, i32 1, i32 1}
// CHECK-DAG: [[MD7]] = !{i32 4}
// CHECK-DAG: [[MD8]] = !{i32 3, i32 2, i32 4}
// CHECK-DAG: [[MD9]] = !{i32 1}
// CHECK-DAG: [[MD10]] = !{i1 true}
// CHECK-DAG: [[MD12]] = !{i32 12}
// CHECK-DAG: [[MD13]] = !{i1 false}
