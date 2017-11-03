// RUN: %clang_cc1 -x cl -O0 -cl-std=CL2.0 -triple spir -emit-llvm %s -o - | FileCheck %s

__kernel void
producer (write_only pipe int __attribute__((blocking)) c0) {
    for (int i = 0; i < 10; i++) {
        write_pipe( c0, &i  );
    }
}
// CHECK: define {{.*}} void @producer
// CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* {{.*}}, i32 4, i32 4)

__kernel void
consumer (__global int * restrict dst,
        read_only pipe int __attribute__((blocking)) __attribute__((depth(10))) c0) {
    for (int i = 0; i < 5; i++) {
        read_pipe( c0, &dst[i]  );
    }
}
// CHECK: define {{.*}} void @consumer
// CHECK: %{{[0-9]+}} = {{.*}}call i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{[0-9]+}}, i32 4, i32 4)
