// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=45 -x c++ -triple x86_64-unknown-unknown -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-x86_64-host.bc
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=45 -x c++ -triple x86_64-unknown-unknown -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-x86_64-host.bc -o - | FileCheck %s --check-prefix TCHECK
// expected-no-diagnostics

class X {
  virtual ~X();
};

X::~X() {}

int main() {
  #pragma omp target
  {}
  return 0;
}

// TCHECK-NOT: @_ZTV1X = unnamed_addr constant
