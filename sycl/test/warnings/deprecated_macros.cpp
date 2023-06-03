// RUN: %clangxx %fsycl-host-only -fsyntax-only -Xclang -verify -Xclang -verify-ignore-unexpected=note -DDISABLE_SYCL_INSTRUMENTATION_METADATA %s

#include <sycl/sycl.hpp>

int main() {
  sycl::queue Queue{};

  Queue
      .submit([](sycl::handler &CGH) {
        // expected-warning@sycl/detail/common.hpp:114 {{DISABLE_SYCL_INSTRUMENTATION_METADATA is deprecated.}}
        CGH.parallel_for(sycl::range<1>(1), [=](sycl::id<1> index) {});
      })
      .wait();
  return 0;
}