// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c11 -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void foo()
{
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
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
    // CHECK: DIR.OMP.TASKGROUP
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.TASKGROUP
    #pragma omp taskloop grainsize(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKGROUP
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.GRAINSIZE
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.TASKGROUP
    #pragma omp taskloop grainsize(j+4)
    for(i=0;i<10;i++) {}
  }

  /* num_tasks */
  {
    int i,j = 4;
    // CHECK: DIR.OMP.TASKGROUP
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.TASKGROUP
    #pragma omp taskloop num_tasks(j)
    for(i=0;i<10;i++) {}
    // CHECK: DIR.OMP.TASKGROUP
    // CHECK: DIR.OMP.TASKLOOP
    // CHECK-SAME: QUAL.OMP.NUM_TASKS
    // CHECK: DIR.OMP.END.TASKLOOP
    // CHECK: DIR.OMP.END.TASKGROUP
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

// Check that omp.[iv|lb|ub] remain i32 for normal loops and taskloop
// CHECK-LABEL: @foo2
void foo2(unsigned int N) {
// CHECK:    [[DOTOMP_IV:%.omp.iv.*]] = alloca i32, align 4
// CHECK:    [[DOTOMP_LB:%.omp.lb.*]] = alloca i32, align 4
// CHECK:    [[DOTOMP_UB:%.omp.ub.*]] = alloca i32, align 4
// CHECK:    [[I:%i.*]] = alloca i32, align 4
// CHECK:    [[DOTOMP_IV15:%.omp.iv.*]] = alloca i32, align 4
// CHECK:    [[DOTOMP_LB16:%.omp.lb.*]] = alloca i32, align 4
// CHECK:    [[DOTOMP_UB17:%.omp.ub.*]] = alloca i32, align 4
// CHECK:    [[I22:%i.*]] = alloca i32, align 4

// CHECK: "DIR.OMP.PARALLEL.LOOP"(),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(ptr [[DOTOMP_IV]]),
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[DOTOMP_LB]]),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(ptr [[DOTOMP_UB]]),
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[I]]) ]
// CHECK: "DIR.OMP.END.PARALLEL.LOOP"()
 #pragma omp parallel for
 for (int i=0; i<N; i++)
   foo();

// CHECK: "DIR.OMP.TASKLOOP"(),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(ptr [[DOTOMP_IV15]]),
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[DOTOMP_LB16]]),
// CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(ptr [[DOTOMP_UB17]]),
// CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[I22]]) ]
// CHECK: "DIR.OMP.END.TASKLOOP"()
 #pragma omp taskloop
 for (int i=0; i<N; i++)
   foo();
}

// end INTEL_COLLAB
