// UNSUPPORTED: windows
// UNSUPPORTED: cuda
// CUDA does not support the function pointer as kernel argument extension.

// REQUIRES: cpu
// RUN: %clangxx -fsycl -fsycl-enable-function-pointers %s -o %t.out
// RUN: %{run} %t.out

#include <sycl.hpp>
#include <CL/sycl/INTEL/function_ref_tuned.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

[[intel::device_indirectly_callable]] int add(int A, int B) { return A + B; }

[[intel::device_indirectly_callable]] int sub(int A, int B) { return A - B; }

using namespace sycl::INTEL;

using masked_variant = masked(varying, uniform);
using unmasked_variant = unmasked(varying, uniform);
using sg_sizes = int_list<4, 8>;

using wrapper_type = function_ref_tuned<decltype(add), sg_sizes, masked_variant,
                                        unmasked_variant>;

template <auto F> auto make_function_ref() {
  return make_function_ref_tuned<F, sg_sizes, masked_variant,
                                 unmasked_variant>();
}

int main() {
  const int Size = 100;
  std::vector<long> A(Size, 1);

  sycl::queue Q;

  sycl::buffer<long> BufA(A.data(), sycl::range<1>(Size));

  sycl::buffer<wrapper_type> BufC(sycl::range<1>(2));

  Q.submit([&](sycl::handler &CGH) {
     auto AccC =
         BufC.template get_access<sycl::access::mode::discard_write>(CGH);
     CGH.single_task<class Init>([=]() {
       AccC[0] = make_function_ref<add>();
       AccC[1] = make_function_ref<sub>();
     });
   }).wait();
  Q.submit([&](sycl::handler &CGH) {
    sycl::accessor AccA{BufA, CGH, sycl::read_write};
    sycl::accessor AccC{BufC, CGH, sycl::read_only};
    CGH.parallel_for<class K>(
        sycl::range<1>(Size),
        [=](sycl::id<1> Index) [[intel::reqd_sub_group_size(8)]] {
          AccA[Index] = AccC[Index % 2](AccA[Index], 2);
        });
  });

  sycl::host_accessor HostAcc{BufA, sycl::read_only};
  auto *Data = HostAcc.get_pointer();

  bool Passed = true;
  for (int i = 0; i < Size; ++i) {
    Passed &= (i % 2 == 0) ? (Data[i] == 3) : (Data[i] == -1);
  }
  if (Passed) {
    std::cout << "Test PASSED." << std::endl;
    return 0;
  } else {
    std::cout << "Test FAILED." << std::endl;
    for (int I = 0; I < Size; ++I) {
      std::cout << HostAcc[I] << " ";
    }
    std::cout << std::endl;
    return 1;
  }
}
