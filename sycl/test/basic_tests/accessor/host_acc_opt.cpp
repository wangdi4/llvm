<<<<<<< HEAD
// RUN: %clangxx -O2 -std=c++17 -I %sycl_include/sycl -I %sycl_include -S -emit-llvm %s -o - | FileCheck %s

// The test verifies that the accessor::operator[] implementation is
// good enough for compiler to optimize away calls to getOffset and
// getMemoryRange and vectorize the loop.
=======
// RUN: %clangxx -O2 -isystem %sycl_include/sycl -isystem %sycl_include -S -emit-llvm %s -o - | FileCheck %s

// The test verifies that the accessor::operator[] implementation is
// good enough for compiler to optimize away calls to getOffset and
// getMemoryRange.
>>>>>>> 4b0e9be7e992b0e4cc985e277e4e6d30d0eda728

#include <sycl/sycl.hpp>

// CHECK: define {{.*}}foo{{.*}} {
// CHECK-NOT: call
// CHECK-NOT: invoke
<<<<<<< HEAD
// CHECK: load <4 x i32>
// CHECK-NOT: call
// CHECK-NOT: invoke
// CHECK: store <4 x i32>
// CHECK-NOT: call
// CHECK-NOT: invoke
=======
>>>>>>> 4b0e9be7e992b0e4cc985e277e4e6d30d0eda728
// CHECK: }
void foo(sycl::accessor<int, 1, sycl::access::mode::read_write,
                        sycl::target::host_buffer> &Acc,
         int *Src) {
  for (size_t I = 0; I < 64; ++I)
<<<<<<< HEAD
    Acc[I] = Src[I];
=======
    Acc[I] += Src[I];
>>>>>>> 4b0e9be7e992b0e4cc985e277e4e6d30d0eda728
}
