// RUN: %clang_cc1 -verify -fopenmp -fintel-compatibility -ast-print %s | FileCheck %s
// expected-no-diagnostics

// CHECK-LABEL: int main()
int main() {
  int a, b;
// CHECK: #pragma omp parallel private(a,b)
#pragma omp parallel private(a, b, )
  ;
  return 0;
}
