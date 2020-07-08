<<<<<<< HEAD
// UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang -fsycl-device-only %s -S -emit-llvm -o - | FileCheck %s
=======
// RUN: %clang_cc1 -fsycl -fsycl-is-device -triple spir64-unknown-unknown-sycldevice %s -S -emit-llvm -o - | FileCheck %s
>>>>>>> 9f4212465204294b7e3936d2bd991e3e040f20db

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  // CHECK-NOT: noinline
  kernel_single_task<class kernel_function>([]() {});
  return 0;
}
