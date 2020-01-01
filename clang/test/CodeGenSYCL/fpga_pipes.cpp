<<<<<<< HEAD
// UNSUPPORTED: intel_opencl
// RUN: %clang %s -S -emit-llvm -fsycl-device-only -o - | FileCheck %s
=======
// RUN: %clang_cc1 %s -emit-llvm -triple spir64-unknown-unknown-sycldevice -fsycl-is-device -disable-llvm-passes -o - | FileCheck %s
>>>>>>> 756deb8037eee668a42aefde8d3e0c6c31faa14f
// CHECK: %opencl.pipe_wo_t
// CHECK: %opencl.pipe_ro_t

using WPipeTy = __attribute__((pipe("write_only"))) const int;
WPipeTy WPipeCreator();

using RPipeTy = __attribute__((pipe("read_only"))) const int;
RPipeTy RPipeCreator();

template <typename PipeTy>
void foo(PipeTy Pipe) {}

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  kernel_single_task<class kernel_function>([]() {
    // CHECK: alloca %opencl.pipe_wo_t
    WPipeTy wpipe = WPipeCreator();
    // CHECK: alloca %opencl.pipe_ro_t
    RPipeTy rpipe = RPipeCreator();
    foo<WPipeTy>(wpipe);
    foo<RPipeTy>(rpipe);
  });
  return 0;
}

