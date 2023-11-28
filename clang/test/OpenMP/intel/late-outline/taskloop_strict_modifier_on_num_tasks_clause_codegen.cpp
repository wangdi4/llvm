// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -fopenmp-version=51 -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
void foo() {
}

bool foobool(int argc) {
  return argc;
}

template <class T, class S>
int tmain(T argc, S **argv) {
  T z;
  #pragma omp taskloop num_tasks(strict: 10) 
  for (int i = 0; i < 10; ++i)
    foo();
  return 0;
}

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  int z = 1;
  
  // CHECK: "DIR.OMP.MASKED"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  #pragma omp masked taskloop num_tasks(strict: argc)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.MASKED"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  // CHECK: "DIR.OMP.SIMD"()
  #pragma omp masked taskloop simd num_tasks(strict: z+1)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.PARALLEL"()
  // CHECK: "DIR.OMP.MASKED"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  #pragma omp parallel masked taskloop num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.PARALLEL"()
  // CHECK: "DIR.OMP.MASKED"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  // CHECK: "DIR.OMP.SIMD"()
  #pragma omp parallel masked taskloop simd num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.MASTER"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  #pragma omp master taskloop num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.MASTER"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  // CHECK: "DIR.OMP.SIMD"()
  #pragma omp master taskloop simd num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.PARALLEL"()
  // CHECK: "DIR.OMP.MASTER"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  #pragma omp parallel master taskloop num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  // CHECK: "DIR.OMP.PARALLEL"()
  // CHECK: "DIR.OMP.MASTER"()
  // CHECK: "DIR.OMP.TASKLOOP"()
  // CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
  // CHECK: "DIR.OMP.SIMD"()
  #pragma omp parallel master taskloop simd num_tasks(strict: 10)
  for (int i = 0; i < 10; ++i)
    foo();

  return tmain(argc, argv);
}

// CHECK-LABEL: @_Z{{.*}}tmain{{.*}}
// CHECK: "DIR.OMP.TASKGROUP"()
// CHECK-SAME: "QUAL.OMP.IMPLICIT"()
// CHECK: "DIR.OMP.TASKLOOP"()
// CHECK-SAME: "QUAL.OMP.NUM_TASKS:STRICT"
// end INTEL_COLLAB
