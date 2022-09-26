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
  int tid = 0;

  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.PARALLEL
  // check-same: QUAL.OMP.IF should this be here? See 2.14
  // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
  // check-same: QUAL.OMP.REDUCTION.ADD should this be here? See 2.14
  // CHECK-NOT: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.MASKED
  // CHECK-SAME: QUAL.OMP.FILTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.DEFAULT.SHARED
  // CHECK-SAME: QUAL.OMP.UNTIED
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.GRAINSIZE
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASKED
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
  // CHECK-NEXT: DIR.OMP.END.TASKGROUP
#pragma omp taskgroup allocate(d) task_reduction(+: d)
#ifdef COMBINED
#pragma omp parallel masked taskloop filter(tid) if(taskloop: argc > N) default(shared) untied priority(N) grainsize(N) reduction(+:g) allocate(g)
#else
#pragma omp parallel default(shared)
#pragma omp masked taskloop filter(tid) if(taskloop: argc > N) default(shared) untied priority(N) grainsize(N) reduction(+:g) allocate(g)
#endif
  for (int i = 0; i < 2; ++i)
    a = 2;

  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASKED
  // CHECK-SAME: QUAL.OMP.FILTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.FINAL
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.NOGROUP
  // CHECK-SAME: QUAL.OMP.NUM_TASKS
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASKED
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
#pragma omp parallel
#ifdef COMBINED
#pragma omp parallel masked taskloop filter(tid) private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) nogroup num_tasks(N)
#else
#pragma omp parallel
#pragma omp masked taskloop filter(tid) private(argc, b), firstprivate(c, d), lastprivate(d, f) collapse(N) shared(g) if (c) final(d) mergeable priority(f) nogroup num_tasks(N)
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
