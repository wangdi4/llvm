// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED1 %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED2 %s | FileCheck %s

int bar2(int i);

void foo()
{
  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASKED
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: DIR.OMP.END.MASKED
  // CHECK: DIR.OMP.END.PARALLEL
  /* taskloop : collapse */
  {
    int i,j;
#ifdef COMBINED1
    #pragma omp parallel masked taskloop collapse(2)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop collapse(2)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop collapse(2)
#endif
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  /* taskloop : grainsize */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
  // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop grainsize(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop grainsize(j)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop grainsize(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop grainsize(j+4)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop grainsize(j+4)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop grainsize(j+4)
#endif
    for(i=0;i<10;i++) {}
  }

  /* taskloop : num_tasks */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop num_tasks(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop num_tasks(j)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop num_tasks(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop num_tasks(j+4)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop num_tasks(j+4)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop num_tasks(j+4)
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : priority */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop priority(0)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop priority(0)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop priority(0)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop priority(j)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop priority(j)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop priority(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop priority(j+4)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop priority(j+4)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop priority(j+4)
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : final */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop final(0)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop final(0)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop final(0)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop final(1)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop final(1)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop final(1)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop final(j!=4)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop final(j!=4)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop final(j!=4)
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : nogroup */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop nogroup
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop nogroup
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop nogroup
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : mergeable */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop mergeable
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop mergeable
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop mergeable
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : untied */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop untied
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop untied
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop untied
#endif
    for(i=0;i<10;i++) {}
  }
  /* parallel and taskloop : if-clause */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop if (parallel: i < j) if (taskloop: j > 0)
#elif defined(COMBINED2)
    #pragma omp parallel if (parallel: i < j)
    #pragma omp masked taskloop if (taskloop: j > 0)
#else
    #pragma omp parallel if (parallel: i < j)
    #pragma omp masked
    #pragma omp taskloop if (taskloop: j > 0)
#endif
    for(i=0;i<10;i++) {}
  }
  /* taskloop : default, parallel : proc_bind */
  {
    extern void bar(int);
    int i,j = 4;
    int m = 4, z = 3;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"()
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop default(shared)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop default(shared)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop default(shared)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.PROC_BIND.SPREAD
    // CHECK: DIR.OMP.MASKED
    // CHECK-NEXT: fence acquire
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK-SAME: QUAL.OMP.FIRSTPRIVATE
    // CHECK-SAME: QUAL.OMP.LASTPRIVATE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: fence release
    // CHECK-NEXT: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop default(none) private(i) firstprivate(j) lastprivate(m) shared(z) proc_bind(spread)
#elif defined(COMBINED2)
    #pragma omp parallel proc_bind(spread)
    #pragma omp masked taskloop default(none) private(i) firstprivate(j) lastprivate(m) shared(z)
#else
    #pragma omp parallel proc_bind(spread)
    #pragma omp masked
    #pragma omp taskloop default(none) private(i) firstprivate(j) lastprivate(m) shared(z)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z);
    }
  }
  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASKED
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
  /* taskloop : reduction */
  {
    int i,j = 4;
    int r = 0;
#ifdef COMBINED1
    #pragma omp parallel masked taskloop reduction (+:r)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop reduction (+:r)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop reduction (+:r)
#endif
    for(i=0;i<10;i++) {
      r += j;
    }
  }

  /* taskloop : allocate */
  {
    int i;
    int b;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop allocate(b), private(b)
#elif defined(COMBINED2)
    #pragma omp parallel
    #pragma omp masked taskloop allocate(b), private(b)
#else
    #pragma omp parallel
    #pragma omp masked
    #pragma omp taskloop allocate(b), private(b)
#endif
    for(i=0;i<10;i++) {}
  }

  /* parallel : num_threads */
  {
    int i;
    int b;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.NUM_THREADS
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop num_threads(5)
#elif defined(COMBINED2)
    #pragma omp parallel num_threads(5)
    #pragma omp masked taskloop
#else
    #pragma omp parallel num_threads(5)
    #pragma omp masked
    #pragma omp taskloop
#endif
    for(i=0;i<10;i++) {}
  }

  static int afoo;
#pragma omp threadprivate(afoo)
  /* parallel : copy_in */
  {
    int i;
    // CHECK: DIR.OMP.PARALLEL
    // CHECK-SAME: QUAL.OMP.COPYIN
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
    // CHECK: DIR.OMP.END.PARALLEL
#ifdef COMBINED1
    #pragma omp parallel masked taskloop copyin(afoo)
#elif defined(COMBINED2)
    #pragma omp parallel copyin(afoo)
    #pragma omp masked taskloop
#else
    #pragma omp parallel copyin(afoo)
    #pragma omp masked
    #pragma omp taskloop
#endif
    for(i=0;i<10;i++) {}
  }
}
// end INTEL_COLLAB
