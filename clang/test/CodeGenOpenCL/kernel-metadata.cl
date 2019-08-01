// RUN: %clang_cc1 %s -emit-llvm -o - | FileCheck %s

void normal_function() {
}

__kernel void kernel_function() {
}

// if INTEL_CUSTOMIZATION
// CHECK: define {{.*}}spir_kernel void @kernel_function() {{[^{]+}} !kernel_arg_addr_space ![[MD:[0-9]+]] !kernel_arg_access_qual ![[MD]] !kernel_arg_type ![[MD]] !kernel_arg_base_type ![[MD]] !kernel_arg_type_qual ![[MD]] !kernel_arg_host_accessible ![[MD]] !kernel_arg_pipe_depth ![[MD]] !kernel_arg_pipe_io ![[MD]] !kernel_arg_buffer_location ![[MD]] {
// endif INTEL_CUSTOMIZATION
// CHECK: ![[MD]] = !{}
