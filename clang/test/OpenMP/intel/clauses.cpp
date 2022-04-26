// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fexceptions -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu %s | FileCheck %s

int foo();

// CHECK-LABEL: @_Z3barii
// CHECK: [[IF_VAL_ADDR:%.+]] = alloca i32,
// CHECK: [[NUM_THREADS_VAL_ADDR:%.+]] = alloca i32,
void bar(int if_val, int num_threads_val) {
  // CHECK: [[IF1_ADDR:%.+]] = alloca i32,
  int if1 = 1;
  // CHECK: [[IF2_ADDR:%.+]] = alloca i32,
  int if2 = 2;
  // CHECK: [[PB1_ADDR:%.+]] = alloca i32,
  int pb1 = 1;
  // CHECK: [[PB2_ADDR:%.+]] = alloca i32,
  int pb2 = 2;
  // CHECK: [[PB3_ADDR:%.+]] = alloca i32,
  int pb3 = 3;
  // CHECK: [[NT1_ADDR:%.+]] = alloca i32,
  int nt1 = 1;
  // CHECK: [[NT2_ADDR:%.+]] = alloca i32,
  int nt2 = 2;
  // CHECK: [[DF1_ADDR:%.+]] = alloca i32,
  int df1 = 1;
  // CHECK: [[DF2_ADDR:%.+]] = alloca i32,
  int df2 = 2;

  // if
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IF1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.IF"(i1 true)
  #pragma omp parallel private(if1) if(1)
  { foo(); }

  // CHECK: [[ILOAD1:%.+]] = load i32, ptr [[IF_VAL_ADDR]]
  // CHECK-NEXT: [[TOBOOL:%.+]] = icmp ne i32 [[ILOAD1]], 0
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IF2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[TOBOOL]])
  #pragma omp parallel private(if2) if(if_val)
  { foo(); }

  // proc_bind
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.MASTER"
  #pragma omp parallel private(pb1) proc_bind(master)
  { foo(); }

  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.CLOSE"
  #pragma omp parallel private(pb2) proc_bind(close)
  { foo(); }

  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[PB3_ADDR]]
  // CHECK-SAME: "QUAL.OMP.PROC_BIND.SPREAD"
  #pragma omp parallel private(pb3) proc_bind(spread)
  { foo(); }

  // num_threads
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[NT1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 8)
  #pragma omp parallel private(nt1) num_threads(8)
  { foo(); }

  // CHECK: [[ILOAD2:%.*]] = load i32, ptr [[NUM_THREADS_VAL_ADDR]]
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[NT2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.NUM_THREADS"(i32 [[ILOAD2]])
  #pragma omp parallel private(nt2) num_threads(num_threads_val)
  { foo(); }

  // default
  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[DF1_ADDR]]
  // CHECK-SAME: "QUAL.OMP.DEFAULT.NONE"
  #pragma omp parallel private(df1) default(none)
  { foo(); }

  // CHECK: "QUAL.OMP.PRIVATE:TYPED"(ptr [[DF2_ADDR]]
  // CHECK-SAME: "QUAL.OMP.DEFAULT.SHARED"
  #pragma omp parallel private(df2) default(shared)
  { foo(); }
}
