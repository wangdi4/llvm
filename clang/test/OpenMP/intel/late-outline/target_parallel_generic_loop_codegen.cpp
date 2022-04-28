// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -include-pch %t -verify -fopenmp-version=50 \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck \
// RUN:  --check-prefixes CHECK,CHECK-NEW %s

// RUN: %clang_cc1 -opaque-pointers -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fno-openmp-new-depend-ir -fopenmp-late-outline -fopenmp-typed-clauses -fopenmp-version=50 \
// RUN:  -triple x86_64-unknown-linux-gnu %s
//
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fno-openmp-new-depend-ir -include-pch %t -verify -fopenmp-version=50 \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck \
// RUN:  --check-prefixes CHECK,CHECK-OLD %s

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

enum omp_allocator_handle_t {
  omp_null_allocator = 0,
  omp_default_mem_alloc = 1,
  omp_large_cap_mem_alloc = 2,
  omp_const_mem_alloc = 3,
  omp_high_bw_mem_alloc = 4,
  omp_low_lat_mem_alloc = 5,
  omp_cgroup_mem_alloc = 6,
  omp_pteam_mem_alloc = 7,
  omp_thread_mem_alloc = 8,
  KMP_ALLOCATOR_MAX_HANDLE = __UINTPTR_MAX__
};

int aaa[1000];
int bbb[1000];
int y = 42;
#pragma omp threadprivate(y)

//CHECK-LABEL: foo1
void foo1(int ploop) {
  int i;
  int z = 6;
  int x;
  int pr;

  //CHECK-DAG: [[I:%i[0-9]*]] = alloca i32
  //CHECK-DAG: [[IV:%.omp.iv[0-9]*]] = alloca i32
  //CHECK-DAG: [[LB:%.omp.lb[0-9]*]] = alloca i32
  //CHECK-DAG: [[UB:%.omp.ub[0-9]*]] = alloca i32
  //CHECK-DAG: [[Z:%z[0-9]*]] = alloca i32
  //CHECK-DAG: [[X:%x[0-9]*]] = alloca i32
  //CHECK-DAG: [[CCC:%ccc[0-9]*]] = alloca [100 x i32]
  //CHECK-DAG: [[PR:%pr[0-9]*]] = alloca i32

  //CHECK: "DIR.OMP.TARGET"(),
  //CHECK-DAG: "QUAL.OMP.DEVICE"(i32 0),
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[PR]],
  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-DAG: "QUAL.OMP.PRIVATE:TYPED"(ptr [[CCC]]
  //CHECK-DAG: "QUAL.OMP.IF"
  //CHECK-DAG: "QUAL.OMP.PROC_BIND.MASTER"
  //CHECK-DAG: "QUAL.OMP.NUM_THREADS"(i32 16),
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.PARALLEL"()
  //CHECK-DAG: "QUAL.OMP.ORDER.CONCURRENT"()
  //CHECK-DAG: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[I]]
  //CHECK-DAG: "QUAL.OMP.SHARED:TYPED"(ptr @aaa
  //CHECK-DAG: "QUAL.OMP.SHARED:TYPED"(ptr @bbb
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV]]
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[UB]]
  //CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LB]]
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  //CHECK: "DIR.OMP.END.PARALLEL"()
  //CHECK: "DIR.OMP.END.TARGET"()

  int ccc[100];
  #pragma omp target parallel loop private(ccc) collapse(2) bind(parallel)    \
                                   copyin(y) device(0) map(tofrom:pr)         \
                                   reduction(+:x)                             \
                                   uses_allocators(omp_default_mem_alloc)     \
                                   allocate(omp_default_mem_alloc:z)          \
                                   if(ploop) proc_bind(master) num_threads(16)\
                                   firstprivate(z) order(concurrent)          \
                                   lastprivate(i)
  for (i=0; i<1000; ++i)
  for (int j=0; j<1000; ++j) {
    ccc[i] = i + j;
    aaa[i] = bbb[i] + ccc[i] + i + z;
    x = i+z;
    pr = pr + 33;
  }

  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-DAG: "QUAL.OMP.DEFAULT.NONE"()
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  #pragma omp target parallel loop default(none) private(ccc)
  for (i=0; i<1000; ++i) {
    ccc[i] = 0;
  }
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  //CHECK: "DIR.OMP.END.PARALLEL"()

}

//CHECK-LABEL: task_target
void task_target() {
  int i;
  short y = 3;
  //CHECK-DAG: [[I:%i[0-9]*]] = alloca i32
  //CHECK-DAG: [[Y:%.+]] = alloca i16,
  //CHECK-NEW-DAG: [[DARR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  //CHECK: DIR.OMP.TASK
  //CHECK-DAG: "QUAL.OMP.IF"(i32 0)
  //CHECK-DAG: "QUAL.OMP.TARGET.TASK"
  //CHECK-OLD-DAG: "QUAL.OMP.DEPEND.OUT"(ptr [[Y]])
  //CHECK-NEW-DAG: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DARR]])
  //CHECK: DIR.OMP.TARGET
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[Y]],
  //CHECK: DIR.OMP.PARALLEL
  //CHECK: DIR.OMP.GENERICLOOP
  //CHECK: DIR.OMP.END.GENERICLOOP
  //CHECK: DIR.OMP.END.PARALLEL
  //CHECK: DIR.OMP.END.TARGET
  //CHECK: DIR.OMP.END.TASK
  #pragma omp target parallel loop map(tofrom:y) depend(out:y)
  for (i=0; i<10; ++i) {
    y = 3+i;
  }
}
#endif
// end INTEL_COLLAB
