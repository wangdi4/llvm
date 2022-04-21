// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes -fcxx-exceptions \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fexceptions \
// RUN:  -fopenmp-late-outline -fintel-compatibility -std=c++14 \
// RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

// RUN: %clang_cc1 -opaque-pointers -triple spir64 \
// RUN:  -emit-llvm -disable-llvm-passes -fcxx-exceptions \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fexceptions \
// RUN:  -fopenmp-late-outline -fintel-compatibility -std=c++14 \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
// RUN:  -verify -Wsource-uses-openmp -o - %s \
// RUN:  | FileCheck %s

// expected-no-diagnostics

void foo(int a) {
  const int b = 100;
  int matrix[a][b];
  if (a) {
#pragma omp target map(alloc: matrix[a][0:b])
    for (int i = 0; i < b; ++i)
      matrix[a][i] += 1;
  } else {
    throw 10;
  }
}
// CHECK: define {{.*}} spir_func void @_Z3fooi(i32 noundef %a)
// CHECK:  call spir_func void @__cxa_throw(ptr addrspace(4) {{.+}}, ptr addrspace(4) {{.+}}, ptr addrspace(4) null)
// CHECK: declare spir_func void @__cxa_throw(ptr addrspace(4), ptr addrspace(4), ptr addrspace(4))

// end INTEL_COLLAB
