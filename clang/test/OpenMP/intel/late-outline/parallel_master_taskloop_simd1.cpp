// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -DCOMBINED %s | FileCheck %s

void foo() {}

template <class T, int N>
T tmain(T argc) {
  T b = argc, c, d, e, f, g;
  static T a;

  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
  // check-same: QUAL.OMP.REDUCTION.ADD should this be here? See 2.14
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
  // CHECK-SAME: QUAL.OMP.UNTIED
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.GRAINSIZE
  // check-same: QUAL.OMP.REDUCTION.ADD same here about reduction
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK-SAME: QUAL.OMP.SIMDLEN
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
  // CHECK-NEXT: DIR.OMP.END.TASKGROUP
#pragma omp taskgroup allocate(d) task_reduction(+: d)
#ifdef COMBINED
#pragma omp parallel master taskloop simd if(taskloop: argc > N) default(shared) untied priority(N) grainsize(N) reduction(+:g) allocate(g) simdlen(8)
#else
#pragma omp parallel default(shared)
#pragma omp master taskloop simd if(taskloop: argc > N) default(shared) untied priority(N) grainsize(N) reduction(+:g) allocate(g) simdlen(8)
#endif
  for (int i = 0; i < 2; ++i)
    a = 2;

  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.FINAL
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.NOGROUP
  // CHECK-SAME: QUAL.OMP.NUM_TASKS
  // CHECK: DIR.OMP.SIMD
  // Should be here once we switch to OpenMP5.0 (allows if on simd)
  // CHECK-NOT: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.SAFELEN
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
#ifdef COMBINED
#pragma omp parallel master taskloop simd private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) nogroup num_tasks(N) safelen(8)
#else
#pragma omp parallel if (c)
#pragma omp master taskloop simd private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) nogroup num_tasks(N) safelen(8)
#endif
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      for (int j = 0; j < 2; ++j)
        for (int j = 0; j < 2; ++j)
          for (int j = 0; j < 2; ++j)
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      for (int j = 0; j < 2; ++j)
        for (int j = 0; j < 2; ++j)
          for (int j = 0; j < 2; ++j)
            foo();
  return T();
}

int main(int argc, char **argv) {
  int b = argc, c, d, e, f, g;
  static int a;

  return (tmain<int, 5>(argc) + tmain<char, 1>(argv[0][0]));
}
// end INTEL_COLLAB
