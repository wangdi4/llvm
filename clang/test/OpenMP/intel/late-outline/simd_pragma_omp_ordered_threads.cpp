// INTEL_COLLAB

// Tests whether we get an assertion when we try to run multiple ordered regions
// in a thread.

// RUN: %clang_cc1 -verify -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -Wuninitialized %s

void bar() {
  #define N 1024
  int t, A[N], B[N], C[N] = {0};

#pragma omp for ordered
  for (int i = 0; i < 1024; i++) {
    A[i] = B[i] + C[i];
// expected-note@+1 {{previous 'ordered' directive used here}}
#pragma omp ordered
    t = A[i];
// expected-error@+1 {{exactly one 'ordered' directive must appear in the loop body of an enclosing directive}}
#pragma omp ordered
    B[i] = t;
  }
}
// end INTEL_COLLAB
