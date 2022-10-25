<<<<<<< HEAD
// RUN: %clangxx -std=c++17 -O2 -isystem %sycl_include/sycl -isystem %sycl_include -S -emit-llvm %s -o - | FileCheck %s
=======
// RUN: %clangxx -O2 -isystem %sycl_include/sycl -isystem %sycl_include -S -emit-llvm %s -o - | FileCheck %s
>>>>>>> 4b0e9be7e992b0e4cc985e277e4e6d30d0eda728

// The test verifies that the accessor::operator[] implementation is
// good enough for compiler to optimize away calls to getOffset and
// getMemoryRange.

#include <sycl/sycl.hpp>

// CHECK: define {{.*}}foo{{.*}} {
// CHECK-NOT: call
// CHECK-NOT: invoke
// CHECK: }
void foo(sycl::accessor<int, 1, sycl::access::mode::read_write,
                        sycl::target::host_buffer> &Acc,
         int *Src) {
  for (size_t I = 0; I < 64; ++I)
    Acc[I] += Src[I];
}
