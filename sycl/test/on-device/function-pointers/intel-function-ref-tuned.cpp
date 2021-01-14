// UNSUPPORTED: windows
// UNSUPPORTED: cuda
// CUDA does not support the function pointer as kernel argument extension.

// RUN: %clangxx -fsycl -fsycl-enable-function-pointers %s -o %t.out
// FIXME: at the moment execution of the test is disabled, because not all
// pieces of the whole implementation are ready.
// RUNx: %CPU_RUN_PLACEHOLDER %t.out

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/function_ref_tuned.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

[[intel::device_indirectly_callable]] int add(int A, int B) { return A + B; }

[[intel::device_indirectly_callable]] int sub(int A, int B) { return A - B; }

using wrapper_type = cl::sycl::INTEL::function_ref_tuned<
    decltype(add), cl::sycl::INTEL::int_list<4, 8>,
    cl::sycl::INTEL::masked(cl::sycl::INTEL::varying, cl::sycl::INTEL::linear),
    cl::sycl::INTEL::unmasked(cl::sycl::INTEL::uniform,
                              cl::sycl::INTEL::uniform)>;

int main() {
  const int Size = 10;
  std::vector<long> A(Size, 1);
  std::vector<long> B(Size, 2);

  cl::sycl::queue Q;

  cl::sycl::buffer<long> BufA(A.data(), cl::sycl::range<1>(Size));
  cl::sycl::buffer<long> BufB(B.data(), cl::sycl::range<1>(Size));

  cl::sycl::buffer<wrapper_type> BufC(cl::sycl::range<1>(2));

  Q.submit([&](cl::sycl::handler &CGH) {
     auto AccC =
         BufC.template get_access<cl::sycl::access::mode::discard_write>(CGH);
     CGH.single_task<class Init>([=]() {
       AccC[0] = cl::sycl::INTEL::make_function_ref_tuned<
           add, cl::sycl::INTEL::int_list<4, 8>,
           cl::sycl::INTEL::masked(cl::sycl::INTEL::varying,
                                   cl::sycl::INTEL::linear),
           cl::sycl::INTEL::unmasked(cl::sycl::INTEL::uniform,
                                     cl::sycl::INTEL::uniform)>();
       AccC[1] = cl::sycl::INTEL::make_function_ref_tuned<
           sub, cl::sycl::INTEL::int_list<4, 8>,
           cl::sycl::INTEL::masked(cl::sycl::INTEL::varying,
                                   cl::sycl::INTEL::linear),
           cl::sycl::INTEL::unmasked(cl::sycl::INTEL::uniform,
                                     cl::sycl::INTEL::uniform)>();
     });
   }).wait();
  Q.submit([&](cl::sycl::handler &CGH) {
    auto AccA =
        BufA.template get_access<cl::sycl::access::mode::read_write>(CGH);
    auto AccB = BufB.template get_access<cl::sycl::access::mode::read>(CGH);
    auto AccC = BufC.template get_access<cl::sycl::access::mode::read>(CGH);
    CGH.parallel_for<class K>(
        cl::sycl::range<1>(Size), [=
    ](cl::sycl::id<1> Index) [[intel::reqd_sub_group_size(8)]] {
          AccA[Index] = AccC[Index % 2](AccA[Index], AccB[Index]);
        });
  });

  auto HostAcc = BufA.get_access<cl::sycl::access::mode::read>();
  auto *Data = HostAcc.get_pointer();

  bool Passed = true;
  for (int i = 0; i < Size; ++i) {
    Passed &= (i % 2 == 0) ? (Data[i] == 3) : (Data[i] == -1);
  }
  if (Passed) {
    std::cout << "Test PASSED." << std::endl;
  } else {
    std::cout << "Test FAILED." << std::endl;
    for (int I = 0; I < Size; ++I) {
      std::cout << HostAcc[I] << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}

