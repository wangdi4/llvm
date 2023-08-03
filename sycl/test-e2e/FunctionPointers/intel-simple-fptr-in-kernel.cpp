// UNSUPPORTED: windows
// UNSUPPORTED: cuda
// CUDA does not support the function pointer as kernel argument extension.

// REQUIRES: cpu
// RUN: %clangxx -fsycl -fsycl-enable-function-pointers %s -o %t.out
// RUN: %{run} %t.out

#include <sycl.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

[[intel::device_indirectly_callable]] int add(int A, int B) { return A + B; }

[[intel::device_indirectly_callable]] int sub(int A, int B) { return A - B; }

int main() {
  const int Size = 100;
  std::vector<long> A(Size, 1);
  std::vector<long> B(Size, 2);

  sycl::queue Q;

  sycl::buffer<long> BufA(A.data(), sycl::range<1>(Size));
  sycl::buffer<long> BufB(B.data(), sycl::range<1>(Size));

  Q.submit([&](sycl::handler &CGH) {
    sycl::accessor AccA{BufA, CGH, sycl::read_write};
    sycl::accessor AccB{BufB, CGH, sycl::read_only};
    CGH.parallel_for<class K>(sycl::range<1>(Size), [=](sycl::id<1> Index) {
      int (*Fptr)(int, int) = nullptr;
      if (Index % 2 == 0)
        Fptr = add;
      else
        Fptr = sub;
      AccA[Index] = Fptr(AccA[Index], AccB[Index]);
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
  } else {
    std::cout << "Test FAILED." << std::endl;
    for (int I = 0; I < Size; ++I) {
      std::cout << HostAcc[I] << " ";
    }
    std::cout << std::endl;
    return 1;
  }

  return 0;
}
