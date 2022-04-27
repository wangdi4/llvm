// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - %s -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu | \
// RUN: FileCheck --check-prefixes CHECK,CHECK-NEW %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - %s -fopenmp -fintel-compatibility -fopenmp-late-outline -triple x86_64-unknown-linux-gnu -fno-openmp-new-depend-ir | \
// RUN: FileCheck --check-prefixes CHECK,CHECK-OLD %s

void foo1() {
  // CHECK: [[X:%.+]] = alloca i32,
  // CHECK: [[A:%.+]] = alloca [5 x i32],
  // CHECK: [[B:%.+]] = alloca float,
  // CHECK: [[C:%.+]] = alloca double,
  int x, a[5]; float b; double c;
  // CHECK: [[AI:%.+]] = getelementptr{{.*}}[[A]]
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-DAG: "QUAL.OMP.MAP.TO:ALWAYS"(ptr [[C]],
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[X]],
  // CHECK-DAG: "QUAL.OMP.MAP.TO"(ptr [[B]],
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[A]], ptr [[AI]], i64 4, i64 0
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  #pragma omp target enter data map(alloc:x) \
                                map (alloc:a[0:1]) map(to:b) map(always,to:c)
}

void foo2() {
  int b;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target enter data nowait map(to:b)
}

void foo3() {
  // CHECK: [[X1:%.+]] = alloca i32,
  // CHECK: [[Y1:%.+]] = alloca float,
  // CHECK: [[B1:%.+]] = alloca double,
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  int x1; float y1; double b1;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-OLD-SAME:"QUAL.OMP.DEPEND.IN"(ptr [[X1]])
  // CHECK-OLD-SAME:"QUAL.OMP.DEPEND.OUT"(ptr [[Y1]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 2, ptr [[DARR]])
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[B1]],
  // CHECK: DIR.OMP.END.TARGET.ENTER.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target enter data depend(in:x1) depend(out:y1) map(to:b1)
}

void foo4() {
  // CHECK: [[X2:%.+]] = alloca i32,
  // CHECK: [[Y2:%.+]] = alloca float,
  // CHECK: [[B2:%.+]] = alloca double,
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  int x2; float y2; double b2;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.IN"(ptr [[X2]])
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.OUT"(ptr [[Y2]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 2, ptr [[DARR]])
  // CHECK: DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[B2]],
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
  // CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[X5]], ptr [[X5]], i64 4, i64 0
  // CHECK-DAG: "QUAL.OMP.MAP.DELETE"(ptr [[A5]], ptr [[AI5]], i64 4, i64 8
  // CHECK-DAG: "QUAL.OMP.MAP.FROM"(ptr [[B5]],
  // CHECK-DAG: "QUAL.OMP.MAP.FROM:ALWAYS"(ptr [[C5]],
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
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[B6]], ptr [[B6]], i64 4, i64 2
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  #pragma omp target exit data nowait map(from:b6)
}

void foo7() {
  // CHECK: [[X7:%.+]] = alloca i32,
  // CHECK: [[Y7:%.+]] = alloca float,
  // CHECK: [[B7:%.+]] = alloca double,
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  int x7; float y7; double b7;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.IN"(ptr [[X7]])
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.OUT"(ptr [[Y7]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 2, ptr [[DARR]])
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[B7]],
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target exit data depend(in:x7) depend(out:y7) map(from:b7)
}

void foo8() {
  // CHECK: [[X8:%.+]] = alloca i32,
  // CHECK: [[Y8:%.+]] = alloca float,
  // CHECK: [[B8:%.+]] = alloca double,
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr{{.*}}, i64 0, i64 0
  int x8; float y8; double b8;
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME "QUAL.OMP.TARGET.TASK"
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.IN"(ptr [[X8]])
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.OUT"(ptr [[Y8]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 2, ptr [[DARR]])
  // CHECK: DIR.OMP.TARGET.EXIT.DATA
  // CHECK-SAME: "QUAL.OMP.NOWAIT"()
  // CHECK-SAME: "QUAL.OMP.MAP.FROM"(ptr [[B8]],
  // CHECK: DIR.OMP.END.TARGET.EXIT.DATA
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target exit data depend(in:x8) depend(out:y8) nowait map(from:b8)
}
