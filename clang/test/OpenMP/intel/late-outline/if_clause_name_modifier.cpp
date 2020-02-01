// INTEL_COLLAB
// Verify if-clause with directive name modifier is applied to only the
// the selected directive.
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void foo()
{
#define N 10
#define NUM_ATTEMPTS 10

  int i, j, a[N] = {0};

  // if (parallel:<expr>)
  // CHECK: DIR.OMP.TARGET
  // CHECK-NOT: QUAL.OMP.IF
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.END.PARALLEL
  // CHECK: DIR.OMP.END.TARGET
  for (i=0; i<NUM_ATTEMPTS * 2; i++) {
#pragma omp target parallel if(parallel:i>=NUM_ATTEMPTS) map(from:a[:N])
    for (j=0; j<N; j++) {
      a[j] = 1;
    }
  }

  // if (target:<expr>)
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-NOT: QUAL.OMP.IF
  // CHECK: DIR.OMP.END.PARALLEL
  // CHECK: DIR.OMP.END.TARGET
  for (i=0; i<NUM_ATTEMPTS * 2; i++) {
#pragma omp target parallel if(target:i>=NUM_ATTEMPTS) map(from:a[:N])
    for (j=0; j<N; j++) {
      a[j] = 1;
    }
  }

  // if (target:<expr>) if (parallel:<expr>)
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.END.PARALLEL
  // CHECK: DIR.OMP.END.TARGET
  for (i=0; i<NUM_ATTEMPTS * 2; i++) {
#pragma omp target parallel if (target: j<10) if(parallel:i>=NUM_ATTEMPTS) map(from:a[:N])
    for (j=0; j<N; j++) {
      a[j] = 1;
    }
  }

  // if (<expr>)
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.END.PARALLEL
  // CHECK: DIR.OMP.END.TARGET
  for (i=0; i<NUM_ATTEMPTS * 2; i++) {
#pragma omp target parallel if (j<10) map(from:a[:N])
    for (j=0; j<N; j++) {
      a[j] = 1;
    }
  }
}
// end INTEL_COLLAB
