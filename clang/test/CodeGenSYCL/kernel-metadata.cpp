// RUN: %clang_cc1 -fsycl -fsycl-is-device -I %S/Inputs -triple spir64-unknown-unknown-sycldevice -emit-llvm %s -o - | FileCheck %s

<<<<<<< HEAD
// CHECK: define {{.*}}spir_kernel void @_ZTSZ4mainE15kernel_function{{.*}} !kernel_arg_addr_space ![[MDAS:[0-9]+]] !kernel_arg_access_qual ![[MDAC:[0-9]+]] !kernel_arg_type ![[MDAT:[0-9]+]] !kernel_arg_base_type ![[MDAT:[0-9]+]] !kernel_arg_type_qual ![[MDATQ:[0-9]+]]
// INTEL_CUSTOMIZATION
// CHECK-NOT: !kernel_arg_host_accessible
// CHECK-NOT: !kernel_arg_pipe_depth
// CHECK-NOT: !kernel_arg_pipe_io
// INTEL_CUSTOMIZATION
// CHECK: ![[MDAS]] = !{i32 1, i32 0, i32 0, i32 0}
// CHECK: ![[MDAC]] = !{!"none", !"none", !"none", !"none"}
// CHECK: ![[MDAT]] = !{!"int*", !"cl::sycl::range<1>", !"cl::sycl::range<1>", !"cl::sycl::id<1>"}
// CHECK: ![[MDATQ]] = !{!"", !"", !"", !""}
=======
// CHECK-NOT: define {{.*}}spir_kernel void @{{.*}}kernel_function{{.*}} !kernel_arg_addr_space {{.*}} !kernel_arg_access_qual {{.*}} !kernel_arg_type {{.*}} !kernel_arg_base_type {{.*}} !kernel_arg_type_qual {{.*}}
>>>>>>> f6589188959027372353c32a45ba904bd7a58f83

#include "sycl.hpp"

int main() {
  cl::sycl::accessor<int, 1, cl::sycl::access::mode::read_write> accessorA;
  cl::sycl::kernel_single_task<class kernel_function>(
      [=]() {
        accessorA.use();
      });
  return 0;
}
