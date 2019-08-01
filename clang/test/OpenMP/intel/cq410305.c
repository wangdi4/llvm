// RUN: %clang_cc1 -verify -fopenmp -fintel-compatibility %s

int main() {
#pragma omp declare target // expected-warning {{ignoring unexpected OpenMP directive '#pragma omp declare target'}}
  int a, b;
#pragma omp end declare target // expected-warning {{ignoring unexpected OpenMP directive '#pragma omp end declare target'}}

  return 0;
}
