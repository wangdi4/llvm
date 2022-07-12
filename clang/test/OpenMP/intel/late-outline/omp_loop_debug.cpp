// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -disable-llvm-passes    \
// RUN:  -debug-info-kind=limited -dwarf-version=4                \
// RUN:  -mllvm -debug-line-version=2 %s | FileCheck %s

void foo() {
  constexpr int N = 100;
  float v3[N], v1[N], v2[N];


  // CHECK: omp.inner.for.body:
  // CHECK: br label %omp.body.continue, !dbg [[DI:![0-9]+]]
  // CHECK: omp.body.continue:
  // CHECK-NEXT: br label %omp.inner.for.inc, !dbg [[DI]]
  int i; /* before-kernel-launch */
#pragma omp parallel for
  for (i = 0; i < N; ++i) {
    v2[i] *= 2;
    v3[i] = v1[i] + v2[i];
  }
}

// end INTEL_COLLAB
