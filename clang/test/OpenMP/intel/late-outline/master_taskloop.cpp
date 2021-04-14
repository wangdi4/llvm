// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED %s | FileCheck %s

int bar2(int i);

void foo()
{
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  /* collapse allowed */
  {
    int i,j;
#ifdef COMBINED
    #pragma omp master taskloop collapse(2)
#else
    #pragma omp master
    #pragma omp taskloop collapse(2)
#endif
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  // grainsize
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop grainsize(j)
#else
    #pragma omp master
    #pragma omp taskloop grainsize(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop grainsize(j+4)
#else
    #pragma omp master
    #pragma omp taskloop grainsize(j+4)
#endif
    for(i=0;i<10;i++) {}
  }

  // num_tasks
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop num_tasks(j)
#else
    #pragma omp master
    #pragma omp taskloop num_tasks(j)
#endif
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop num_tasks(j+4)
#else
    #pragma omp master
    #pragma omp taskloop num_tasks(j+4)
#endif
    for(i=0;i<10;i++) {}
  }

  // priority
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop priority(j)
#else
    #pragma omp master
    #pragma omp taskloop priority(j)
#endif
    for(i=0;i<10;i++) {}
  }

  // final
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop final(j!=4)
#else
    #pragma omp master
    #pragma omp taskloop final(j!=4)
#endif
    for(i=0;i<10;i++) {}
  }

  // nogroup
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop nogroup
#else
    #pragma omp master
    #pragma omp taskloop nogroup
#endif
    for(i=0;i<10;i++) {}
  }

  // mergeable
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop mergeable
#else
    #pragma omp master
    #pragma omp taskloop mergeable
#endif
    for(i=0;i<10;i++) {}
  }

  // untied
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop untied
#else
    #pragma omp master
    #pragma omp taskloop untied
#endif
    for(i=0;i<10;i++) {}
  }

  // if-clause
  {
    int i,j = 4;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.IF
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop if (i < j)
#else
    #pragma omp master
    #pragma omp taskloop if (i < j)
#endif
    for(i=0;i<10;i++) {}
  }

  // default (shared|none)
  {
    extern void bar(int);
    int i,j = 4;
    int m = 4, z = 3;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"()
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop default(shared)
#else
    #pragma omp master
    #pragma omp taskloop default(shared)
#endif
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }

    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK-SAME: QUAL.OMP.FIRSTPRIVATE
    // CHECK-SAME: QUAL.OMP.LASTPRIVATE
    // CHECK-SAME: QUAL.OMP.SHARED
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop default(none) private(i) firstprivate(j) \
                                       lastprivate(m) shared(z)
#else
    #pragma omp master
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
  // CHECK: DIR.OMP.MASTER
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: DIR.OMP.END.MASTER
  /* reduction */
  {
    int i,j = 4;
    int r = 0;
#ifdef COMBINED
    #pragma omp master taskloop reduction (+:r)
#else
    #pragma omp master
    #pragma omp taskloop reduction (+:r)
#endif
    for(i=0;i<10;i++) {
      r += j;
    }
  }

  // in_reduction
  //
  // CHECK: DIR.OMP.MASTER
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASTER
  int n1=0;
  #pragma omp taskgroup task_reduction(+:n1)
  /* in_reduction */
  {
#ifdef COMBINED
    #pragma omp master taskloop in_reduction(+:n1)
#else
    #pragma omp master
    #pragma omp taskloop in_reduction(+:n1)
#endif
    for(int i=0;i<20;++i)
      n1 += bar2(1);
  }

  // allocate
  {
    int i;
    int b;
    // CHECK: DIR.OMP.MASTER
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIVATE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
    #pragma omp master taskloop allocate(b), private(b)
#else
    #pragma omp master
    #pragma omp taskloop allocate(b), private(b)
#endif
    for(i=0;i<10;i++) {}
  }
}
// end INTEL_COLLAB
