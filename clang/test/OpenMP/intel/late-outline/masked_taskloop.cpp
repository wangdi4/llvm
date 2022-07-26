// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED %s | FileCheck %s

int bar2(int i);

void foo()
{
  int tid = 0;
  // CHECK: DIR.OMP.MASKED
  // CHECK-SAME: QUAL.OMP.FILTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASKED
  /* collapse allowed */
  {
    int i,j;
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) collapse(2)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop collapse(2)
#endif
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  // grainsize
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop grainsize(j)
#else
    #pragma omp masked
    #pragma omp taskloop grainsize(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) grainsize(j+4)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop grainsize(j+4)
#endif
    for(i=0;i<10;i++) {}
  }

  // num_tasks
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) num_tasks(j)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop num_tasks(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) num_tasks(j+4)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop num_tasks(j+4)
#endif
    for(i=0;i<10;i++) {}
  }

  // priority
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) priority(j)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop priority(j)
#endif
    for(i=0;i<10;i++) {}
  }

  // final
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) final(j!=4)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop final(j!=4)
#endif
    for(i=0;i<10;i++) {}
  }

  // nogroup
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) nogroup
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop nogroup
#endif
    for(i=0;i<10;i++) {}
  }

  // mergeable
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) mergeable
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop mergeable
#endif
    for(i=0;i<10;i++) {}
  }

  // untied
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) untied
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop untied
#endif
    for(i=0;i<10;i++) {}
  }

  // if-clause
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) if (i < j)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop if (i < j)
#endif
    for(i=0;i<10;i++) {}
  }

  // default (shared|none)
  {
    extern void bar(int);
    int i,j = 4;
    int m = 4, z = 3;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"()
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) default(shared)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop default(shared)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }

    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK-SAME: QUAL.OMP.FIRSTPRIVATE
    // CHECK-SAME: QUAL.OMP.LASTPRIVATE
    // CHECK-SAME: QUAL.OMP.SHARED
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) default(none) private(i) \
                       firstprivate(j) lastprivate(m) shared(z)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop default(none) private(i) firstprivate(j) \
                                       lastprivate(m) shared(z)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z);
    }
  }

  // reduction
  //
  // CHECK: DIR.OMP.MASKED
  // CHECK-SAME: QUAL.OMP.FILTER
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: DIR.OMP.END.MASKED
  /* reduction */
  {
    int i,j = 4;
    int r = 0;
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) reduction (+:r)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop reduction (+:r)
#endif
    for(i=0;i<10;i++) {
      r += j;
    }
  }

  // in_reduction
  //
  // CHECK: DIR.OMP.MASKED
  // CHECK-SAME: QUAL.OMP.FILTER
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASKED
  int n1=0;
  #pragma omp taskgroup task_reduction(+:n1)
  /* in_reduction */
  {
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) in_reduction(+:n1)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop in_reduction(+:n1)
#endif
    for(int i=0;i<20;++i)
      n1 += bar2(1);
  }

  // allocate
  {
    int i;
    int b;
    // CHECK: DIR.OMP.MASKED
    // CHECK-SAME: QUAL.OMP.FILTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASKED
#ifdef COMBINED
    #pragma omp masked taskloop filter(tid) allocate(b), private(b)
#else
    #pragma omp masked filter(tid)
    #pragma omp taskloop allocate(b), private(b)
#endif
    for(i=0;i<10;i++) {}
  }
}
// end INTEL_COLLAB
