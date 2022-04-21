// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
// RUN:  -include-pch %t -verify \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

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

  //CHECK-DAG: [[I:%i[0-9]*]] = alloca i32
  //CHECK-DAG: [[IV:%.omp.iv[0-9]*]] = alloca i32
  //CHECK-DAG: [[LB:%.omp.lb[0-9]*]] = alloca i32
  //CHECK-DAG: [[UB:%.omp.ub[0-9]*]] = alloca i32
  //CHECK-DAG: [[Z:%z[0-9]*]] = alloca i32
  //CHECK-DAG: [[X:%x[0-9]*]] = alloca i32
  //CHECK-DAG: [[CCC:%ccc[0-9]*]] = alloca [100 x i32]

  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-DAG: "QUAL.OMP.PRIVATE"(ptr [[CCC]])
  //CHECK-DAG: "QUAL.OMP.IF"
  //CHECK-DAG: "QUAL.OMP.PROC_BIND.MASTER"
  //CHECK-DAG: "QUAL.OMP.NUM_THREADS"(i32 16),
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.PARALLEL"()
  //CHECK-DAG: "QUAL.OMP.ORDER.CONCURRENT"()
  //CHECK-DAG: "QUAL.OMP.LASTPRIVATE"(ptr [[I]])
  //CHECK-DAG: "QUAL.OMP.SHARED"(ptr @aaa)
  //CHECK-DAG: "QUAL.OMP.SHARED"(ptr @bbb)
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.IV"(ptr [[IV]])
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.UB"(ptr [[UB]])
  //CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE"(ptr [[LB]])
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  //CHECK: "DIR.OMP.END.PARALLEL"()

  int ccc[100];
  #pragma omp parallel loop private(ccc) collapse(2) bind(parallel) copyin(y) \
                            reduction(+:x) allocate(omp_default_mem_alloc:z)  \
                            if(ploop) proc_bind(master) num_threads(16)       \
                            firstprivate(z) order(concurrent) lastprivate(i)
  for (i=0; i<1000; ++i)
  for (int j=0; j<1000; ++j) {
    ccc[i] = i + j;
    aaa[i] = bbb[i] + ccc[i] + i + z;
    x = i+z;
  }

  //CHECK: "DIR.OMP.PARALLEL"(),
  //CHECK-DAG: "QUAL.OMP.DEFAULT.NONE"()
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  #pragma omp parallel loop default(none) private(ccc)
  for (i=0; i<1000; ++i) {
    ccc[i] = 0;
  }
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  //CHECK: "DIR.OMP.END.PARALLEL"()

}
#endif
// end INTEL_COLLAB
