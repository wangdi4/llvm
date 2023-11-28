// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -std=c11 -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

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

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %var to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type (in).
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 1, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:var)
  {
    bar();
  }

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: %{{.+}} = sub nuw
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %arrayidx{{.*}} to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type.
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 1, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:arr[5:10])
  {
    bar();
  }

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %var to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type (out).
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 3, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(out:var)
  {
    bar();
  }
  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: %{{.+}} = sub nuw
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %arrayidx{{.*}} to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type.
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 3, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(out:arr[5:10])
  {
    bar();
  }

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %var to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type (in).
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 3, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(inout:var)
  {
    bar();
  }

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: %{{.+}} = sub nuw
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %arrayidx{{.*}} to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type.
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 3, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(inout:arr[5:10])
  {
    bar();
  }

  // CHECK: [[DEP_ARR_ADDR:%.*]] = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %var to i64
  // CHECK: [[TMP:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 0

  // Dependency variable, 'var'.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type (in).
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP]], i32 0, i32 2
  // CHECK: store i8 1, ptr [[DEP_TYPE_ADDR]], align 8

  // CHECK: %{{.+}} = sub nuw
  // CHECK: [[DVAR:%.*]] = ptrtoint ptr %arrayidx{{.*}} to i64
  // CHECK: [[TMP2:%.*]] = getelementptr %struct.kmp_depend_info, ptr [[DEP_ARR_ADDR]], i64 1

  // Dependency variable, 'arr'.
  // CHECK: [[DEP_VAR_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP2]], i32 0, i32 0
  // CHECK: store i64 [[DVAR]], ptr [[DEP_VAR_ADDR]], align 8

  // Dependency type.
  // CHECK: [[DEP_TYPE_ADDR:%.*]] = getelementptr inbounds %struct.kmp_depend_info, ptr [[TMP2]], i32 0, i32 2
  // CHECK: store i8 3, ptr [[DEP_TYPE_ADDR]], align 8

  // Number of dependencies.
  // CHECK: store i64 2, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 2, ptr [[DEP_ARR_ADDR]])
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(in:var) depend(inout:arr[5:10])
  {
    bar();
  }

  // CHECK: getelementptr inbounds %struct.kmp_depend_info, {{.*}}, i32 0, i32 0
  // CHECK-NEXT: store i64 0
  // CHECK: getelementptr inbounds %struct.kmp_depend_info, {{.*}}, i32 0, i32 1
  // CHECK-NEXT: store i64 0
  // CHECK: getelementptr inbounds %struct.kmp_depend_info, {{.*}}, i32 0, i32 2
  // CHECK-NEXT: store i8 -128
  // CHECK: store i64 1, ptr %dep.counter.addr{{.*}}, align 8

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.DEPARRAY"(i32 1
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task depend(inout:omp_all_memory)
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
// end INTEL_COLLAB
