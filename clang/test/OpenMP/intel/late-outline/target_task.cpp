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
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i16* [[Y]])
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
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.DEPEND.OUT"(i32* [[Y]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32** [[YP]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[SZ]])
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[Y]])
  // CHECK: [[AI:%.+]] = getelementptr{{.*}}
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(i32** [[YP]], i32** [[YP]], i64 8)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(i32** [[YP]], i32* [[AI]], i64 %{{[0-9]+}})
  // CHECK-SAME: "QUAL.OMP.NOWAIT"
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target  map(tofrom:yp[0:size])  nowait depend(out:y)
   y = 3;
}
// end INTEL_COLLAB
