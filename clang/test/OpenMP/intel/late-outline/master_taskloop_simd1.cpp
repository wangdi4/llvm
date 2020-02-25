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
  T *ptr;
  static T a;

  // CHECK: DIR.OMP.TASKGROUP
  // CHECK: DIR.OMP.MASTER
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
  // CHECK-SAME: QUAL.OMP.UNTIED
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.GRAINSIZE
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK-SAME: QUAL.OMP.FINAL
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.SAFELEN
  // CHECK-SAME: QUAL.OMP.SIMDLEN
  // CHECK-SAME: QUAL.OMP.LINEAR
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASTER
  // CHECK: DIR.OMP.END.TASKGROUP
#ifdef COMBINED
#pragma omp taskgroup task_reduction(+: d) allocate(d)
#pragma omp master taskloop simd allocate(d) if(taskloop: argc > N) default(shared) untied priority(N) safelen(N) simdlen(N) linear(c) aligned(ptr) grainsize(N) reduction(+:g) in_reduction(+: d) mergeable final(d)
#else
#pragma omp taskgroup task_reduction(+: d) allocate(d)
#pragma omp master
#pragma omp taskloop simd allocate(d) if(taskloop: argc > N) default(shared) untied priority(N) safelen(N) simdlen(N) linear(c) aligned(ptr) grainsize(N) reduction(+:g) in_reduction(+: d) mergeable final(d)
#endif
  for (int i = 0; i < 2; ++i)
    a = 2;

  // CHECK: DIR.OMP.MASTER
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
  // CHECK-SAME: QUAL.OMP.SIMDLEN
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.MASTER
#ifdef COMBINED
#pragma omp master taskloop simd private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) simdlen(N) nogroup num_tasks(N)
#else
#pragma omp master
#pragma omp taskloop simd private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) simdlen(N) nogroup num_tasks(N)
#endif
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      for (int j = 0; j < 2; ++j)
        for (int j = 0; j < 2; ++j)
          for (int j = 0; j < 2; ++j)
            foo();

  return T();
}

int main(int argc, char **argv) {
  return (tmain<int, 5>(argc) + tmain<char, 1>(argv[0][0]));
}
// end INTEL_COLLAB
