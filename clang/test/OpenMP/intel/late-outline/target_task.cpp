// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void foo1() {
  double w = 1.0;
  long x = 2;
  short y = 3;
  // CHECK: [[W:%.+]] = alloca double,
  // CHECK: [[X:%.+]] = alloca i64,
  // CHECK: [[Y:%.+]] = alloca i16,
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double* [[W]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[X]])
  // CHECK-SAME: "QUAL.OMP.DEPEND.OUT"(i16* [[Y]])
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(double* [[W]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i64* [[X]])
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i16* [[Y]], i16* [[Y]], i64 2, i64 35)
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target private(w) firstprivate(x) map(tofrom:y) depend(out:y)
    y = 3;
}

void foo2() {
  int y = 2;
  int *yp = &y;
  volatile int size = 1;
  // CHECK: [[Y:%.+]] = alloca i32,
  // CHECK: [[YP:%.+]] = alloca i32*,
  // CHECK: [[SZ:%.+]] = alloca i32,
  // CHECK: [[YPMAP:%.+]] = alloca i32*, align 8
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.DEPEND.OUT"(i32* [[Y]])
  // CHECK-DAG: "QUAL.OMP.SHARED"(i32** [[YP]])
  // CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE"(i32* [[Y]])
  // CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE"(i32* [[SZ]])
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[YPMAP]])
  // CHECK: [[L0:%.+]] = load i32*, i32** [[YP]]
  // CHECK: [[L1:%.+]] = load i32*, i32** [[YP]]
  // CHECK: [[L2:%.+]] = load i32*, i32** [[YP]]
  // CHECK-NEXT: [[AI:%.+]] = getelementptr{{.*}}
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1)
  // CHECK-SAME: "QUAL.OMP.NOWAIT"
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32* [[L1]], i32* [[AI]], i64 %10, i64 35)
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32** [[YPMAP]])
  // CHECK: store i32* [[L1]], i32** [[YPMAP]], align 8
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target  map(tofrom:yp[0:size])  nowait depend(out:y)
   {
      yp[1] = 0;
      y = 3;
   }
}
// end INTEL_COLLAB
