//  SYCL 2020 Conformance Test
//
//  This test calls sycl::link(vector<kernel_bundle<>>, std::vector<device>,
//  property_list) and sycl::link(kernel_bundle<>, std::vector<device>,
//  property_list). For obtained kernel bundle with object state will use
//  one device, but for sycl::link will be provided vector with device, that was
//  used for kernel bundle and different devices.
//
//  The test verifies that an exception with sycl::errc::invalid was thrown.

// RUN: %clangxx -fsycl
// RUN: %s

// expected-no-diagnostics

#include <sycl/sycl.hpp>

class MyKernel;

using vector_with_object_bundles =
    std::vector<sycl::kernel_bundle<sycl::bundle_state::object>>;

int main() {
  const std::vector<sycl::device> devices{sycl::device::get_devices()};
  if (devices.size() <= 1) {
    std::cout << "Test skipped due to only zero or one device was found"
              << std::endl;
    return 0;
  }

  sycl::queue q{devices[0]};

  const auto EmptyKernelId = sycl::get_kernel_id<MyKernel>();
  auto kernel_bundle = sycl::get_kernel_bundle<sycl::bundle_state::object>(
      q.get_context(), {EmptyKernelId});
  vector_with_object_bundles vector_with_kb{kernel_bundle};

  int catchedExceptions = 0;
  try {
    sycl::link(vector_with_kb, devices);
  } catch (sycl::exception &e) {
    assert(e.code() == sycl::errc::invalid);
    catchedExceptions++;
  }
  try {
    sycl::link(kernel_bundle, devices);
  } catch (sycl::exception &e) {
    assert(e.code() == sycl::errc::invalid);
    catchedExceptions++;
  }
  // Exceptions must be generated for both cases
  assert(catchedExceptions == 2);

  q.submit([&](sycl::handler &h) {
    h.single_task<MyKernel>([=] {
      // do nothing
    });
  });

  return 0;
}
