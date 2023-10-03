// INTEL_COLLAB
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu -fopenmp-version=50 %s | FileCheck %s

extern int a;
void foo(int);
void b() {
  int c[5][a];
  //CHECK: [[VLAT:%omp.vla.tmp.*]] = alloca i64,
  //CHECK: store i64 %1, ptr [[VLAT]]
  //CHECK: "DIR.OMP.TARGET"{{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[VLAT]]
  //CHECK: [[VLATL1:%[0-9]*]] = load i64, ptr [[VLAT]]
  //CHECK-NEXT: [[MUL:%[0-9]*]] = mul nuw i64 5, [[VLATL1]]
  //CHECK: "DIR.OMP.TEAMS"{{.*}}"QUAL.OMP.SHARED:TYPED"(ptr [[VLAT]]
  //CHECK: [[VLATL2:%[0-9]*]] = load i64, ptr [[VLAT]]
  //CHECK-NEXT: [[MUL2:%[0-9]*]] = mul nsw i64 3, [[VLATL2]]
  //CHECK: getelementptr inbounds i32, ptr %vla, i64 [[MUL2]]
  #pragma omp target teams
  foo(c[3][2]);
}

// end INTEL_COLLAB
