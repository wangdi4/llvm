// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp -triple x86_64-unknown-linux-gnu | FileCheck %s

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
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[IF1_ADDR]])
  // CHECK-NEXT: opnd.i1(metadata !"QUAL.OMP.IF", i1 true)
  #pragma omp parallel private(if1) if(1)
  { foo(); }

  // CHECK: [[ILOAD1:%.+]] = load i32, i32* [[IF_VAL_ADDR]]
  // CHECK-NEXT: [[TOBOOL:%.+]] = icmp ne i32 [[ILOAD1]], 0
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[IF2_ADDR]])
  // CHECK-NEXT: opnd.i1(metadata !"QUAL.OMP.IF", i1 [[TOBOOL]])
  #pragma omp parallel private(if2) if(if_val)
  { foo(); }

  // proc_bind
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[PB1_ADDR]])
  // CHECK-NEXT: qual(metadata !"QUAL.OMP.PROCBIND.MASTER")
  #pragma omp parallel private(pb1) proc_bind(master)
  { foo(); }

  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[PB2_ADDR]])
  #pragma omp parallel private(pb2) proc_bind(close)
  // CHECK-NEXT: qual(metadata !"QUAL.OMP.PROCBIND.CLOSE")
  { foo(); }

  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[PB3_ADDR]])
  // CHECK-NEXT: qual(metadata !"QUAL.OMP.PROCBIND.SPREAD")
  #pragma omp parallel private(pb3) proc_bind(spread)
  { foo(); }

  // num_threads
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[NT1_ADDR]])
  // CHECK-NEXT: opnd.i32(metadata !"QUAL.OMP.NUM_THREADS", i32 8)
  #pragma omp parallel private(nt1) num_threads(8)
  { foo(); }

  // CHECK: [[ILOAD2:%.*]] = load i32, i32* [[NUM_THREADS_VAL_ADDR]]
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[NT2_ADDR]])
  // CHECK-NEXT: opnd.i32(metadata !"QUAL.OMP.NUM_THREADS", i32 [[ILOAD2]])
  #pragma omp parallel private(nt2) num_threads(num_threads_val)
  { foo(); }

  // default
  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[DF1_ADDR]])
  // CHECK-NEXT: qual(metadata !"QUAL.OMP.DEFAULT.NONE")
  #pragma omp parallel private(df1) default(none)
  { foo(); }

  // CHECK: opndlist(metadata !"QUAL.OMP.PRIVATE", i32* [[DF2_ADDR]])
  // CHECK-NEXT: qual(metadata !"QUAL.OMP.DEFAULT.SHARED")
  #pragma omp parallel private(df2) default(shared)
  { foo(); }
}
