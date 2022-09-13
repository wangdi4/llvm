// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm %s -o - | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --implicit-check-not not_used

#pragma omp begin declare target
constexpr float array[] = { 0.0f, 1.0f };
#pragma omp end declare target
// Verify that both host and target definitions match and are internal
//CHECK: @_ZL5array = internal {{.*}}constant [2 x float]
//CHECK-SAME: [float 0.000000e+00, float 1.000000e+00], align 4

constexpr float not_used[] = { 0.0f, 1.0f };

struct A {
  inline const float& get0() { return array[0]; }
};

void foo()
{
  A avar;
  float f1 = avar.get0();
}

void other ()
{
  float f = not_used[0];
}

int main()
{
  #pragma omp target
  foo();
  return 0;
}
