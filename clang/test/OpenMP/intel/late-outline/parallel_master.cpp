// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED %s | FileCheck %s
//

int bar2(int i);

void foo()
{
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.MASTER
  // CHECK: DIR.OMP.END.MASTER
  // CHECK: DIR.OMP.END.PARALLEL
  /* parallel : if */
  {
    int i,j;
#ifdef COMBINED
    #pragma omp parallel master if (i == j)
#else
    #pragma omp parallel if(i == j)
    #pragma omp master
#endif
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  /* parallel : if (parallel : value) */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
  // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master if (parallel : i < j)
#else
    #pragma omp parallel if (parallel : i < j)
    #pragma omp master
#endif
    for(i=0;i<10;i++) {}
  }

  /* parallel : default */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.DEFAULT
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master default(shared)
#else
    #pragma omp parallel default(shared)
    #pragma omp master
#endif
    for(i=0;i<10;i++) {}

    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.DEFAULT
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master default(none) private(i)
#else
    #pragma omp parallel default(none) private(i)
    #pragma omp master
#endif
    for(i=0;i<10;i++) {}
  }

  static int w;
#pragma omp threadprivate(w)

  /* parallel: private firstprivate shared proc_bind copyin */
  {
    int i,j = 4;
    int m = 1; int z = 10;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK-SAME: QUAL.OMP.FIRSTPRIVATE
    // CHECK-SAME: QUAL.OMP.SHARED
    // CHECK-SAME: QUAL.OMP.PROC_BIND
    // CHECK-SAME: QUAL.OMP.COPYIN
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master private(i) firstprivate(j) shared(z) proc_bind(spread) copyin(w)
#else
    #pragma omp parallel private(i) firstprivate(j) shared(z) proc_bind(spread) copyin(w)
    #pragma omp master
#endif
    for(i=0;i<10;i++) {}

  /* parallel: reduction */
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master reduction(+:m)
#else
    #pragma omp parallel reduction(+:m)
    #pragma omp master
#endif

  /* parallel: private */
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED
    #pragma omp parallel master private(z) allocate(z)
#else
    #pragma omp parallel private(z) allocate(z)
    #pragma omp master
#endif
    for(i=0;i<10;i++) {}
  }
}
// end INTEL_COLLAB
