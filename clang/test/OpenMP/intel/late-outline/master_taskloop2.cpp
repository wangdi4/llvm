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

  return T();
}

int main(int argc, char **argv) {
  int b = argc, c, d, e, f, g;
  static int a;

  // CHECK: DIR.OMP.TASKGROUP
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.MUL
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
  // CHECK-SAME: QUAL.OMP.FINAL
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.NUM_TASKS
  // CHECK-SAME: QUAL.OMP.INREDUCTION.MUL
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK: DIR.OMP.END.TASKGROUP
#pragma omp taskgroup task_reduction(+: d)
#ifdef COMBINED
#pragma omp master taskloop if(taskloop: a) default(none) shared(a) final(b) priority(5) num_tasks(argc) reduction(*: g) in_reduction(+:d)
#else
#pragma omp master
#pragma omp taskloop if(taskloop: a) default(none) shared(a) final(b) priority(5) num_tasks(argc) reduction(*: g) in_reduction(+:d)
#endif
  for (int i = 0; i < 2; ++i)
    a = 2;

  // CHECK: DIR.OMP.PARALLEL
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKGROUP
  // CHECK-SAME: QUAL.OMP.REDUCTION.MAX
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.GRAINSIZE
  // CHECK-SAME: QUAL.OMP.INREDUCTION.MAX
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: DIR.OMP.END.TASKGROUP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK: DIR.OMP.END.PARALLEL
#pragma omp parallel
#ifdef COMBINED
#pragma omp master taskloop private(argc, b), firstprivate(argv, c), lastprivate(d, f) collapse(2) shared(g) if(argc) mergeable priority(argc) grainsize(argc) reduction(max: a, e)
#else
#pragma omp master
#pragma omp taskloop private(argc, b), firstprivate(argv, c), lastprivate(d, f) collapse(2) shared(g) if(argc) mergeable priority(argc) grainsize(argc) reduction(max: a, e)
#endif
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 10; ++j)
      foo();

  return (tmain<int, 5>(argc) + tmain<char, 1>(argv[0][0]));
}
// end INTEL_COLLAB
