// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu -DCOMBINED1 | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu -DCOMBINED2 | FileCheck %s


void foo()
{
  /* collapse allowed */
  {
    int i,j;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.MASTER
    // CHECK-NEXT: fence acquire
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.COLLAPSE
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.SIMD
    // CHECK-SAME: QUAL.OMP.COLLAPSE
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: fence release
    // CHECK-NEXT: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd collapse(2) if (i < j)
#elif defined(COMBINED2)
    #pragma omp parallel if (i < j)
    #pragma omp master taskloop simd collapse(2) if (i < j)
#else
    #pragma omp parallel if (i < j)
    #pragma omp master
    #pragma omp taskloop simd collapse(2) if (i < j)
#endif
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  /* grainsize */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd grainsize(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd grainsize(j)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd grainsize(j)
#endif
    for(i=0;i<10;i++) {}
  }

  /* num_tasks */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd num_tasks(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd num_tasks(j)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd num_tasks(j)
#endif
    for(i=0;i<10;i++) {}
  }
  /* priority */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd priority(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd priority(j)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd priority(j)
#endif
    for(i=0;i<10;i++) {}
  }
  /* final */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd final(1)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd final(1)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd final(1)
#endif
    for(i=0;i<10;i++) {}
  }
  /* nogroup */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd nogroup
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd nogroup
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd nogroup
#endif
    for(i=0;i<10;i++) {}
  }
  /* mergeable */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd mergeable
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd mergeable
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd mergeable
#endif
    for(i=0;i<10;i++) {}
  }
  /* untied */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd untied
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd untied
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd untied
#endif
    for(i=0;i<10;i++) {}
  }
  /* default */
  {
    extern void bar(int);
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd default(shared)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd default(shared)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd default(shared)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }
    int m = 4, z = 3;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK-SAME: QUAL.OMP.SHARED
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK-SAME: QUAL.OMP.FIRSTPRIVATE
    // CHECK-SAME: QUAL.OMP.LASTPRIVATE
    // CHECK-SAME: QUAL.OMP.SHARED
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd default(none) firstprivate(j) \
                                       lastprivate(m) shared(z)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd default(none) firstprivate(j)  lastprivate(m) shared(z)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd default(none) firstprivate(j) \
                                       lastprivate(m) shared(z)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z);
    }
  }
  {
    extern void bar(int);
    int i,j = 4;
    int m = 4;
    int z[10] = {0,1,2,3,4,5,6,7,8,9};
    int *zp = &z[0];
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK: DIR.OMP.SIMD
    // CHECK-SAME: QUAL.OMP.SIMDLEN
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel master taskloop simd safelen(4) simdlen(4) linear(m) aligned(zp)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp master taskloop simd safelen(4) simdlen(4) linear(m) aligned(zp)
#else
    #pragma omp parallel
    #pragma omp master
    #pragma omp taskloop simd safelen(4) simdlen(4) linear(m) aligned(zp)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z[i]);
    }
  }
}
// end INTEL_COLLAB
