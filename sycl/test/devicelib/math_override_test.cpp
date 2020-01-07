// UNSUPPORTED: windows
// RUN: %clangxx -fsycl -c %s -o %t.o
// RUN: %clangxx -fsycl %t.o %llvm_build_libs_dir/libsycl-cmath.o -o %t.out
#include <CL/sycl.hpp>
#include <iostream>
#include <math.h>
namespace s = cl::sycl;
constexpr s::access::mode sycl_read = s::access::mode::read;
constexpr s::access::mode sycl_write = s::access::mode::write;

// Dummy function provided by user to override device library
// version.
SYCL_EXTERNAL
double sin(double x) {
  return x + 100;
}

class DeviceTest;

void device_test() {
  s::queue deviceQueue;
  s::range<1> numOfItems{1};
  double result_sin = 0;
  double result_cos = 0;
  {
    s::buffer<double, 1> buffer1(&result_sin, numOfItems);
    s::buffer<double, 1> buffer2(&result_cos, numOfItems);
    deviceQueue.submit([&](s::handler &cgh) {
      auto res_access_sin = buffer1.get_access<sycl_write>(cgh);
      auto res_access_cos = buffer2.get_access<sycl_write>(cgh);
      cgh.single_task<class DeviceTest>([=]() {
        // Should use the sin function defined by user, device
        // library version should be ignored here
        res_access_sin[0] = sin(0);
        res_access_cos[0] = cos(0);
      });
    });
  }

  assert(((int)result_sin == 100) && ((int)result_cos == 1));
}

int main() {
  device_test();
  std::cout << "Pass" << std::endl;
  return 0;
}
