// RUN: %clang_cc1 -emit-llvm -o - %s -std=c11 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

void foo()
{
  /* collapse allowed */
  {
    int i,j;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK: DIR.OMP.SIMD
    // CHECK-SAME: QUAL.OMP.COLLAPSE
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd collapse(2)
    for(i=0;i<10;i++)
      for(j=0;j<10;j++) {
      }
  }

  /* grainsize */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd grainsize(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd grainsize(j+4)
    for(i=0;i<10;i++) {}
  }

  /* num_tasks */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd num_tasks(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd num_tasks(j+4)
    for(i=0;i<10;i++) {}
  }
  /* priority */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd priority(0)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd priority(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.PRIORITY
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd priority(j+4)
    for(i=0;i<10;i++) {}
  }
  /* final */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd final(0)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd final(1)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.FINAL
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd final(j!=4)
    for(i=0;i<10;i++) {}
  }
  /* nogroup */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NOGROUP
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd nogroup
    for(i=0;i<10;i++) {}
  }
  /* mergeable */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.MERGEABLE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd mergeable
    for(i=0;i<10;i++) {}
  }
  /* untied */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.UNTIED
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd untied
    for(i=0;i<10;i++) {}
  }
  /* default */
  {
    extern void bar(int);
    int i,j = 4;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd default(shared)
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k);
    }
    int m = 4, z = 3;
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
    // CHECK: DIR.OMP.SIMD
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd default(none) firstprivate(j) \
                                       lastprivate(m) shared(z)
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
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK: DIR.OMP.SIMD
    // CHECK-SAME: QUAL.OMP.SIMDLEN
    // CHECK: DIR.OMP.END.SIMD
    // CHECK: DIR.OMP.END.TASKLOOP
    #pragma omp taskloop simd safelen(4) simdlen(4) linear(m) aligned(zp)
    for(i=0;i<10;i++) {
      int k = j + 1;
      bar(i+k+m+z[i]);
    }
  }
}
