// INTEL_FEATURE_CSA
// REQUIRES: csa-registered-target
// RUN: %clang_cc1 -verify -fopenmp -fintel-compatibility -triple csa %s

void foo() {
#pragma omp parallel for dataflow // expected-error {{expected '(' after 'dataflow'}}
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for dataflow() // expected-error {{unexpected 'dataflow' modifier, expected 'static', 'num_workers' or 'pipeline'}}
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for dataflow(invalid0) // expected-error {{unexpected 'dataflow' modifier, expected 'static', 'num_workers' or 'pipeline'}}
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for dataflow(invalid1, static(4)) // expected-error {{unexpected 'dataflow' modifier}}
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for dataflow(static(4),invalid2) // expected-error {{unexpected 'dataflow' modifier}}
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for num_threads(2) dataflow(static(4)) // expected-no-error
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for num_threads(2) dataflow(num_workers(4)) // expected-no-error
  for (int i = 0; i < 16; ++i) {
  }
#pragma omp parallel for dataflow(num_workers(4)) num_threads(2) // expected-no-error
  for (int i = 0; i < 16; ++i) {
  }
}
// end INTEL_FEATURE_CSA
