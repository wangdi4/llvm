// RUN: %clang_cc1 -fopenmp -triple x86_64-unknown-linux -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
// RUN: %clang_cc1 -verify -fopenmp -triple spir64 -aux-triple x86_64-unknown-linux -fopenmp-targets=spir64 %s -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc -fsyntax-only

__bf16 test(); // expected-note {{'test' defined here}}

void foo() {
  auto A = test(); // No error in host code
}

int main()
{
  #pragma omp target
  {
    // expected-error@+1 {{'test' requires 16 bit size '__bf16' type support, but target 'spir64' does not support it}}
    auto B = test();
  }
  return 0;
}

