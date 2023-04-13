// INTEL_COLLAB

// RUN: %clang_cc1 -I%S/Inputs -emit-llvm -triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-threadprivate-legacy \
// RUN:  -verify -o - %s | FileCheck %s

// expected-no-diagnostics

#include <omp.h>

void use(int);

int threadprivate_value;
#pragma omp threadprivate(threadprivate_value)

// CHECK: define{{.*}}foo
void foo() {

  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel
  {
    threadprivate_value = omp_get_thread_num();

    // CHECK: DIR.OMP.TASK{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
    #pragma omp task
    {
      use(threadprivate_value);
    }
    // CHECK: DIR.OMP.END.TASK

    // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
    #pragma omp taskloop
    for (int i=0; i < 1; ++i) {
      use(threadprivate_value);
    }
    // CHECK: DIR.OMP.END.TASKLOOP

    // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
    // CHECK: DIR.OMP.SIMD
    #pragma omp taskloop simd
    for (int i=0; i < 1; ++i) {
      use(threadprivate_value);
    }
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP

    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
    #pragma omp masked taskloop
    for (int i=0; i < 1; ++i) {
      use(threadprivate_value);
    }
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED

    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
    // CHECK: DIR.OMP.SIMD
    #pragma omp masked taskloop simd
    for (int i=0; i < 1; ++i) {
      use(threadprivate_value);
    }
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
  }
  // CHECK: DIR.OMP.END.PARALLEL

  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASKED
  // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
  #pragma omp parallel masked taskloop
  for (int i=0; i < 1; ++i) {
    use(threadprivate_value);
  }
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASKED
  // CHECK: DIR.OMP.END.PARALLEL

  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASKED
  // CHECK: DIR.OMP.TASKLOOP{{.*}}"QUAL.OMP.LIVEIN"(ptr @threadprivate_value)
  // CHECK: DIR.OMP.SIMD
  #pragma omp parallel masked taskloop simd
  for (int i=0; i < 1; ++i) {
    use(threadprivate_value);
  }
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASKED
  // CHECK: DIR.OMP.END.PARALLEL
}
// end INTEL_COLLAB
