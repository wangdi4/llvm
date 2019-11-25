// UNSUPPORTED: intel_opencl
<<<<<<< HEAD
// RUN: %clang %s -S -emit-llvm -fsycl-device-only -o - | FileCheck %s
=======
// RUN: %clang %s -S -emit-llvm --sycl -o - | FileCheck %s
>>>>>>> a8c278025b9bb77f252e23aa51025f3630750ca6
#include "CL/sycl.hpp"
// CHECK: %opencl.pipe_wo_t
// CHECK: %opencl.pipe_ro_t

class SomePipe;
void foo() {
  using Pipe = cl::sycl::pipe<SomePipe, int>;
  // CHECK: %{{.*}} = alloca %opencl.pipe_wo_t
  Pipe::write(42);
  // CHECK: %{{.*}} = alloca %opencl.pipe_ro_t
  int a = Pipe::read();
}

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  kernel_single_task<class kernel_function>([]() {
    foo();
  });
  return 0;
}

