// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu --gnu_fabi_version=6 -std=c++0x -emit-llvm -o - %s | FileCheck %s

template <class... Args> int f(Args... args);

// CHECK: define {{.*}} @_Z1gIJidEEDTcl1fspplfp_Li1EEEDpT_
template <class... Args> auto g(Args... args) -> decltype(f((args + 1)...)) {
  return (f((args + 1)...));
}

void test() { g(42, 1.0); }
