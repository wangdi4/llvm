// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// Verify task/default reduction modifiers are accepted and generate the
// correct IR. Default modifier IR should be identical to no modifier.

void foo(int n) {
  int task_a(0);
  int default_a(0);
  int b[n];

  // CHECK: "DIR.OMP.PARALLEL.LOOP"
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:TASK"(ptr %task_a)
  // CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for reduction(task, + : task_a)
  for(int i = 0; i < n; i++) {
    task_a += b[i] + 1;
  }
  // CHECK: "DIR.OMP.PARALLEL.LOOP"
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(ptr %default_a)
  // CHECK: "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for reduction(default, + : default_a)
  for(int i = 0; i < n; i++) {
    default_a += b[i] + 1;
  }
  return;
}
// end INTEL_COLLAB
