// INTEL_FEATURE_ESIMD_EMBARGO
// REQUIRES: intel_feature_esimd_embargo
// UNSUPPORTED: linux
// RUN: %clangxx -fsycl -c -fsycl-device-only -Xclang -emit-llvm %s -o %t

#include <CL/sycl.hpp>
#include <sycl/ext/intel/experimental/esimd.hpp>

using namespace sycl::ext::intel::experimental::esimd;

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(Func kernelFunc) {
  kernelFunc();
}

void caller(int x) {
  kernel<class kernel_esimd>([=]() SYCL_ESIMD_KERNEL {
   esimd_nbarrier_init<7>();
   esimd_nbarrier_wait(2);
   esimd_nbarrier_signal(0, 0, 4, 4);
  });
}
// end INTEL_FEATURE_ESIMD_EMBARGO
