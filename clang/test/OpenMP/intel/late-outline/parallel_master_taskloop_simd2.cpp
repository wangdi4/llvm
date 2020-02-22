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
  // CHECK-SAME: QUAL.OMP.REDUCTION.ADD
  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // check-same: QUAL.OMP.REDUCTION.MUL should this be here? See 2.14
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.DEFAULT.NONE
  // check-same: QUAL.OMP.REDUCTION.MUL same here about reduction
  // CHECK: DIR.OMP.SIMD
  // CHECK-SAME: QUAL.OMP.REDUCTION.MUL
  // CHECK-SAME: QUAL.OMP.ALIGNED
  // CHECK-SAME: QUAL.OMP.LINEAR
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
  // CHECK-NEXT: DIR.OMP.END.TASKGROUP
#pragma omp taskgroup task_reduction(+: d)
#ifdef COMBINED
#pragma omp parallel master taskloop simd if(parallel: a) default(none) shared(a, b, argc) final(b) priority(5) num_tasks(argc) reduction(*: g) aligned(argv: 8) linear(c:b)
#else
#pragma omp parallel if (a)
#pragma omp master taskloop simd default(none) shared(a, b, argc) final(b) priority(5) num_tasks(argc) reduction(*: g) aligned(argv: 8) linear(c:b)
#endif
  for (int i = 0; i < 2; ++i)
    a = 2;

  // CHECK: DIR.OMP.PARALLEL
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.REDUCTION.MAX
  // CHECK: DIR.OMP.MASTER
  // CHECK-NEXT: fence acquire
  // CHECK: DIR.OMP.TASKLOOP
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK-SAME: QUAL.OMP.GRAINSIZE
  // CHECK-SAME: QUAL.OMP.REDUCTION.MAX
  // CHECK: DIR.OMP.SIMD
  // Should be here once we switch to OpenMP5.0 (allows if on simd)
  // CHECK-NOT: QUAL.OMP.IF
  // CHECK-SAME: QUAL.OMP.COLLAPSE
  // CHECK-SAME: QUAL.OMP.REDUCTION.MAX
  // CHECK: DIR.OMP.END.SIMD
  // CHECK: DIR.OMP.END.TASKLOOP
  // CHECK: fence release
  // CHECK-NEXT: DIR.OMP.END.MASTER
  // CHECK-NEXT: DIR.OMP.END.PARALLEL
#ifdef COMBINED
#pragma omp parallel master taskloop simd private(argc, b), firstprivate(argv, c), lastprivate(d, f) collapse(2) shared(g) if(argc) mergeable priority(argc) grainsize(argc) reduction(max: a, e)
#else
#pragma omp parallel if (argc) reduction(max: a, e)
#pragma omp master taskloop simd private(argc, b), firstprivate(argv, c), lastprivate(d, f) collapse(2) shared(g) if(argc) mergeable priority(argc) grainsize(argc) reduction(max: a, e)
#endif
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 10; ++j)
      foo();

  return (tmain<int, 5>(argc) + tmain<char, 1>(argv[0][0]));
}
// end INTEL_COLLAB
