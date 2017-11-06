// RUN: %clang_cc1 -emit-llvm -o - %s -std=c11 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

void foo()
{
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK: DIR.OMP.END.TASKLOOP
  /* collapse allowed */
  {
    int i,j;
    #pragma omp taskloop collapse(2)
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  /* grainsize */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop grainsize(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop grainsize(j+4)
    for(i=0;i<10;i++) {}
  }

  /* num_tasks */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop num_tasks(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop num_tasks(j+4)
    for(i=0;i<10;i++) {}
  }
  /* priority */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop priority(0)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop priority(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop priority(j+4)
    for(i=0;i<10;i++) {}
  }
  /* final */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop final(0)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop final(1)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop final(j!=4)
    for(i=0;i<10;i++) {}
  }
  /* nogroup */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop nogroup
    for(i=0;i<10;i++) {}
  }
  /* mergeable */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop mergeable
    for(i=0;i<10;i++) {}
  }
  /* untied */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop untied
    for(i=0;i<10;i++) {}
  }
  /* default */
  {
    extern void bar(int);
    int i,j = 4;
    int m = 4, z = 3;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop default(shared)
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop default(none) private(i) firstprivate(j) \
                                       lastprivate(m) shared(z)
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z);
    }
  }
}
