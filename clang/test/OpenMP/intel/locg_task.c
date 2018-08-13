// RUN: %clang_cc1 -emit-llvm -o - %s -std=c11 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s

void bar();

// CHECK-LABEL: @foo
// CHECK: [[VAR:%var.*]] = alloca i32,
// CHECK: [[ARR:%arr.*]] =  alloca [100 x i32],
void foo(int ifval, int finalval, int priorityval)
{
  int var = 1;
  int arr[100] = {0};

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.IF
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task if(ifval)
  {
    bar();
  }

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.FINAL
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task final(finalval)
  {
    bar();
  }

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.PRIORITY
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task priority(priorityval)
  {
    bar();
  }

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.MERGEABLE
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task mergeable
  {
    bar();
  }

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.IN{{.*}}[[VAR]]
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:var)
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.IN:ARRSECT{{.*}}[[ARR]],{{.*}}1,{{.*}}5,{{.*}}10,{{.*}}1)
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:arr[5:10])
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.OUT{{.*}}[[VAR]]
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(out:var)
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.OUT:ARRSECT{{.*}}[[ARR]],{{.*}}1,{{.*}}5,{{.*}}10,{{.*}}1)
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(out:arr[5:10])
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.INOUT{{.*}}[[VAR]]
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(inout:var)
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: QUAL.OMP.DEPEND.INOUT:ARRSECT{{.*}}[[ARR]],{{.*}}1,{{.*}}5,{{.*}}10,{{.*}}1)
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(inout:arr[5:10])
  {
    bar();
  }
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME:  QUAL.OMP.DEPEND.IN{{.*}}[[VAR]]{{.*}}QUAL.OMP.DEPEND.INOUT:ARRSECT{{.*}}[[ARR]],{{.*}}1,{{.*}}5,{{.*}}10,{{.*}}1)
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:var) depend(inout:arr[5:10])
  {
    bar();
  }

  // CHECK: DIR.OMP.PARALLEL
  #pragma omp parallel
  {
    // CHECK: DIR.OMP.TASKGROUP
    #pragma omp taskgroup
    {
      // CHECK: DIR.OMP.TASK
      #pragma omp task
      {
        bar();
      }
      // CHECK: DIR.OMP.END.TASK
      // CHECK: DIR.OMP.TASK
      #pragma omp task
      {
        bar();
      }
      // CHECK: DIR.OMP.END.TASK
    }
    // CHECK: DIR.OMP.END.TASKGROUP
    // CHECK: DIR.OMP.TASKYIELD
    // CHECK: DIR.OMP.END.TASKYIELD
    #pragma omp taskyield
    // CHECK: DIR.OMP.TASKWAIT
    // CHECK: fence acq_rel
    // CHECK: DIR.OMP.END.TASKWAIT
    #pragma omp taskwait
  }
  // CHECK: DIR.OMP.END.PARALLEL
}
