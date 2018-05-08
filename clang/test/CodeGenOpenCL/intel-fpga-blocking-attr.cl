// RUN: %clang_cc1 -x cl -O0 -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,CL20
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s -check-prefixes=CHECK,CL12

__kernel void
producer (write_only pipe int __attribute__((blocking)) c0) {
    for (int i = 0; i < 10; i++) {
        write_pipe( c0, &i  );
    }
}
// CHECK: define {{.*}} void @producer
// CL20: %{{[0-9]+}} = call i32 @__write_pipe_2_bl(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(4)* {{.*}}, i32 4, i32 4)
// CL12: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_AS0(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8* {{.*}}, i32 4, i32 4)

__kernel void
consumer (__global int * restrict dst,
        read_only pipe int __attribute__((blocking)) __attribute__((depth(10))) c0) {
    for (int i = 0; i < 5; i++) {
        read_pipe( c0, &dst[i]  );
    }
}
// CHECK: define {{.*}} void @consumer
// CL20: %{{[0-9]+}} = {{.*}}call i32 @__read_pipe_2_bl(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
// CL12: %{{[0-9]+}} = {{.*}}call i32 @__read_pipe_2_bl_AS1(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(1)* %{{[0-9]+}}, i32 4, i32 4)
