// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-pch -o %t -std=c++14 -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s

// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -std=c++14 -fopenmp -fopenmp-late-outline \
// RUN:  -include-pch %t -verify \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// expected-no-diagnostics
#ifndef HEADER
#define HEADER

int aaa[1000];
//CHECK-LABEL: foo1
void foo1() {
  int i;
  //CHECK-DAG: [[I:%i[0-9]*]] = alloca i32
  //CHECK-DAG: [[IV:%.omp.iv[0-9]*]] = alloca i32
  //CHECK-DAG: [[LB:%.omp.lb[0-9]*]] = alloca i32
  //CHECK-DAG: [[UB:%.omp.ub[0-9]*]] = alloca i32
  //CHECK-DAG: [[Z:%z[0-9]*]] = alloca i32
  //CHECK-DAG: [[X:%x[0-9]*]] = alloca i32
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.PARALLEL"()
  //CHECK-DAG: "QUAL.OMP.ORDER.CONCURRENT"()
  //CHECK-DAG: "QUAL.OMP.PRIVATE"(i32* [[I]])
  //CHECK-DAG: "QUAL.OMP.SHARED"([1000 x i32]* @aaa)
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.IV"(i32* [[IV]])
  //CHECK-DAG: "QUAL.OMP.NORMALIZED.UB"(i32* [[UB]])
  //CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE"(i32* [[LB]])
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  #pragma omp loop bind(parallel) order(concurrent)
  for (i=0; i<1000; ++i) {
    aaa[i] = i;
  }

  // Check for each clause

  int z;
  int x;
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.TEAMS"()
  //CHECK-DAG: "QUAL.OMP.LASTPRIVATE"(i32* [[I]])
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  #pragma omp target teams
  #pragma omp loop bind(teams) private(z) lastprivate(i)
  for (i=0; i<1000; ++i) {
    z = i+11;
    x = i+z;
    aaa[i] = i + z;
  }

  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.THREAD"()
  //CHECK-DAG: "QUAL.OMP.COLLAPSE"(i32 2)
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  #pragma omp loop bind(thread) collapse(2)
  for (i=0; i<1000; ++i)
  for (int j=0; j<10; ++j) {
    aaa[i] = i+j;
  }

  int j;
  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.THREAD"()
  //CHECK-DAG: "QUAL.OMP.COLLAPSE"(i32 2)
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  #pragma omp loop bind(thread) collapse(2) lastprivate(i,j)
  for (i=0; i<1000; ++i)
  for (j=0; j<10; ++j) {
    aaa[i] = i+j;
  }

  //CHECK: "DIR.OMP.GENERICLOOP"(),
  //CHECK-DAG: "QUAL.OMP.BIND.THREAD"()
  //CHECK: "DIR.OMP.END.GENERICLOOP"()
  #pragma omp loop bind(thread) reduction(+:x)
  for (i=0; i<1000; ++i) {
    x += i;
  }

  #pragma omp parallel
  {
    #pragma omp loop reduction(+:x)
    for (i=0; i<1000; ++i) {
      x += i;
    }
  }
}
#endif
// end INTEL_COLLAB
