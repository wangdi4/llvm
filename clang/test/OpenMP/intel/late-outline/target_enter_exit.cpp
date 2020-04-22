// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void foo1() {
  // CHECK: [[X:%.+]] = alloca i32,
  // CHECK: [[A:%.+]] = alloca [5 x i32],
  // CHECK: [[B:%.+]] = alloca float,
  // CHECK: [[C:%.+]] = alloca double,
  int x, a[5]; float b; double c;
  // CHECK: [[AI:%.+]] = getelementptr{{.*}}[[A]]
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(i32* [[X]], i32* [[X]], i64 4, i64 32)
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"([5 x i32]* [[A]], i32* [[AI]], i64 4, i64 32)
  // CHECK-DAG: "QUAL.OMP.MAP.TO"(float* [[B]], float* [[B]], i64 4, i64 33),
  // CHECK-DAG: "QUAL.OMP.MAP.TO:ALWAYS"(double* [[C]], double* [[C]], i64 8, i64 37)
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  #pragma omp target enter data map(alloc:x) \
                                map (alloc:a[0:1]) map(to:b) map(always,to:c)
}

void foo2() {
  int b;
  // CHECK: [[B:%.+]] = alloca i32,
  // CHECK: "DIR.OMP.TASK"()
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"()
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[B]])
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  #pragma omp target enter data nowait map(to:b)
}

void foo3() {
  // CHECK: [[X1:%.+]] = alloca i32,
  // CHECK: [[Y1:%.+]] = alloca float,
  // CHECK: [[B1:%.+]] = alloca double,
  int x1; float y1; double b1;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME:"QUAL.OMP.DEPEND.IN"(i32* [[X1]])
  // CHECK-SAME:"QUAL.OMP.DEPEND.OUT"(float* [[Y1]])
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(double* [[B1]], double* [[B1]], i64 8, i64 33)
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target enter data depend(in:x1) depend(out:y1) map(to:b1)
}

void foo4() {
  // CHECK: [[X2:%.+]] = alloca i32,
  // CHECK: [[Y2:%.+]] = alloca float,
  // CHECK: [[B2:%.+]] = alloca double,
  int x2; float y2; double b2;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.DEPEND.IN"(i32* [[X2]])
  // CHECK-SAME: "QUAL.OMP.DEPEND.OUT"(float* [[Y2]])
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(double* [[B2]], double* [[B2]], i64 8, i64 33)
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target enter data depend(in:x2) depend(out:y2) nowait map(to:b2)
}

void foo5() {
  // CHECK: [[X5:%.+]] = alloca i32,
  // CHECK: [[A5:%.+]] = alloca [2 x i32],
  // CHECK: [[B5:%.+]] = alloca float,
  // CHECK: [[C5:%.+]] = alloca double,
  int x5, a5[2]; float b5; double c5;
  // CHECK: [[AI5:%.+]] = getelementptr{{.*}}[[A5]]
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(i32* [[X5]], i32* [[X5]], i64 4, i64 32)
  // CHECK-DAG: "QUAL.OMP.MAP.DELETE"([2 x i32]* [[A5]], i32* [[AI5]], i64 4, i64 40)
  // CHECK-DAG: "QUAL.OMP.MAP.FROM"(float* [[B5]], float* [[B5]], i64 4, i64 34)
  // CHECK-DAG: "QUAL.OMP.MAP.FROM:ALWAYS"(double* [[C5]], double* [[C5]], i64 8, i64 38)
  //
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  #pragma omp target exit data map(release:x5) map(delete:a5[0:1]) \
                               map(from:b5) map(always,from:c5)
}

void foo6() {
  // CHECK: [[B6:%.+]] = alloca i32,
  int b6;
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(i32* [[B6]], i32* [[B6]], i64 4, i64 34)
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  #pragma omp target exit data nowait map(from:b6)
}

void foo7() {
  // CHECK: [[X7:%.+]] = alloca i32,
  // CHECK: [[Y7:%.+]] = alloca float,
  // CHECK: [[B7:%.+]] = alloca double,
  int x7; float y7; double b7;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME:"QUAL.OMP.DEPEND.IN"(i32* [[X7]])
  // CHECK-SAME:"QUAL.OMP.DEPEND.OUT"(float* [[Y7]])
  // CHECK-SAME:"QUAL.OMP.SHARED"(double* [[B7]])
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(double* [[B7]], double* [[B7]], i64 8, i64 34)
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target exit data depend(in:x7) depend(out:y7) map(from:b7)
}

void foo8() {
  // CHECK: [[X8:%.+]] = alloca i32,
  // CHECK: [[Y8:%.+]] = alloca float,
  // CHECK: [[B8:%.+]] = alloca double,
  int x8; float y8; double b8;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.DEPEND.IN"(i32* [[X8]])
  // CHECK-SAME: "QUAL.OMP.DEPEND.OUT"(float* [[Y8]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(double* [[B8]])
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(double* [[B8]], double* [[B8]], i64 8, i64 34)
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  #pragma omp target exit data depend(in:x8) depend(out:y8) nowait map(from:b8)
}
// end INTEL_COLLAB
