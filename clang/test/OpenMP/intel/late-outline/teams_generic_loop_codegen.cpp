// INTEL_COLLAB
// RUN: %clang_cc1 -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
// RUN:  -include-pch %t -verify \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s -check-prefix TARG

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
//CHECK-LABEL: foo1
//TARG-LABEL: foo1
void foo1() {
  int i,j,x,z;
  //CHECK-DAG: [[I:%i[0-9]*]] = alloca i32
  //CHECK-DAG: [[J:%j[0-9]*]] = alloca i32
  //CHECK-DAG: [[IV:%.omp.iv[0-9]*]] = alloca i32
  //CHECK-DAG: [[LB:%.omp.lb[0-9]*]] = alloca i32
  //CHECK-DAG: [[UB:%.omp.ub[0-9]*]] = alloca i32
  //CHECK-DAG: [[Z:%z[0-9]*]] = alloca i32
  //CHECK-DAG: [[X:%x[0-9]*]] = alloca i32

  //CHECK: "DIR.OMP.TARGET"(),
  //CHECK: "DIR.OMP.TEAMS"(),
  //CHECK-DAG: "QUAL.OMP.NUM_TEAMS"(i32 8)
  //CHECK-DAG: "QUAL.OMP.THREAD_LIMIT"(i32 4)
  //CHECK-DAG: "QUAL.OMP.SHARED:TYPED"(ptr @aaa
  //CHECK-DAG: "QUAL.OMP.DEFAULT.NONE"()
  //CHECK-DAG: "QUAL.OMP.ALLOCATE"(i64 4, ptr [[Z]], i64 1)
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.TEAMS"()
  //CHECK-DAG: "QUAL.OMP.ORDER.CONCURRENT"()
  //CHECK-DAG: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[I]]
  //CHECK-DAG: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[J]]
  //CHECK-DAG: "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr [[X]]
  //CHECK-DAG: "QUAL.OMP.PRIVATE:TYPED"(ptr [[Z]]
  //CHECK-DAG: "QUAL.OMP.SHARED:TYPED"(ptr @aaa
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[IV]]
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[UB]]
  //CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[LB]]
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  //CHECK: "DIR.OMP.END.TEAMS"()
  //CHECK: "DIR.OMP.END.TARGET"()

  //TARG: "DIR.OMP.TARGET"(),
  //TARG: "DIR.OMP.TEAMS"(),
  //TARG: "DIR.OMP.GENERICLOOP"(),
  //TARG-DAG: "QUAL.OMP.BIND.TEAMS"()
  //TARG-DAG: "QUAL.OMP.ORDER.CONCURRENT"()
  //TARG: "DIR.OMP.END.GENERICLOOP"()
  //TARG: "DIR.OMP.END.TEAMS"()
  //TARG: "DIR.OMP.END.TARGET"()

  #pragma omp target
  #pragma omp teams loop bind(teams) order(concurrent) private(z)      \
                         num_teams(8) thread_limit(4) default(none)    \
                         shared(aaa) allocate(omp_default_mem_alloc:z) \
                         lastprivate(i,j) collapse(2) reduction(+:x)
  for (i=0; i<1000; ++i)
  for (j=0; j<10; ++j) {
    z = i+11;
    aaa[i] = i+j+z;
    x += i+j;
  }
}
#endif
// end INTEL_COLLAB
