<<<<<<< HEAD
// RUN: %clang -fsycl-device-only %s -S -emit-llvm -o - | FileCheck %s
=======
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang --sycl %s -S -emit-llvm -o - | FileCheck %s
>>>>>>> b53e0fd964396f29a0c8d3c263a02a96d63be3fb

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  // CHECK-NOT: noinline
  kernel_single_task<class kernel_function>([]() {});
  return 0;
}
