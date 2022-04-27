// RUN: %clang_cc1 -x cl -O0 -cl-std=CL2.0 -triple spir -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,SPIR_20
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -triple spir-unknown-unknown-intelfpga -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,SPIR_12
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL2.0 -triple x86_64 -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,X86
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -triple x86_64-unknown-unknown-intelfpga -emit-llvm -opaque-pointers %s -o - | FileCheck %s -check-prefixes=CHECK,X86

__kernel void
producer (write_only pipe int __attribute__((blocking)) c0) {
    for (int i = 0; i < 10; i++) {
        write_pipe( c0, &i  );
    }
}
// CHECK: define {{.*}} void @producer
// SPIR_20: %{{[0-9]+}} = call spir_func i32 @__write_pipe_2_bl(ptr addrspace(1) %{{.*}}, ptr addrspace(4) {{.*}}, i32 4, i32 4)
// SPIR_12: %{{[0-9]+}} = call spir_func i32 @__write_pipe_2_bl_AS0(ptr addrspace(1) %{{.*}}, ptr {{.*}}, i32 4, i32 4)
// X86: %{{[0-9]+}} = call i32 @__write_pipe_2_bl(ptr %{{.*}}, ptr {{.*}}, i32 4, i32 4)

__kernel void
consumer (__global int * restrict dst,
        read_only pipe int __attribute__((blocking)) __attribute__((depth(10))) c0) {
    for (int i = 0; i < 5; i++) {
        read_pipe( c0, &dst[i]  );
    }
}
// CHECK: define {{.*}} void @consumer
// SPIR_20: %{{[0-9]+}} = {{.*}}call spir_func i32 @__read_pipe_2_bl(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{[0-9]+}}, i32 4, i32 4)
// SPIR_12: %{{[0-9]+}} = {{.*}}call spir_func i32 @__read_pipe_2_bl_AS1(ptr addrspace(1) %{{.*}}, ptr addrspace(1) %{{.+}}, i32 4, i32 4)
// X86: %{{[0-9]+}} = {{.*}}call i32 @__read_pipe_2_bl(ptr %{{.*}}, ptr %{{.+}}, i32 4, i32 4)
