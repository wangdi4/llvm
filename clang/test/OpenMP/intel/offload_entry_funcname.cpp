// RUN: %clang_cc1 -verify -fopenmp -fopenmp-targets=csa -S %s -o - | FileCheck %s

// expected-no-diagnostics
int main() {
  int i;
#pragma omp target  device(0)
  {
    i = 10;
  }
}

// CHECK: .omp_offloading.main.entry:
