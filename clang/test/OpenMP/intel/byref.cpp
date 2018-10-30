// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu %s | FileCheck %s
// RUN: %clang_cc1 -emit-llvm -o - -fexceptions -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu %s | FileCheck %s

#define LOOP for(int i=0;i<16;++i) {}

int foo(int);

// CHECK-LABEL: bar1
void bar1(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[L0:%[0-9]+]] = load i32*, i32** [[DADDR]], align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:BYREF"(i32* [[L0]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar2
void bar2(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[L0:%[0-9]+]] = load i32*, i32** [[DADDR]], align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:BYREF"(i32* [[L0]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for firstprivate(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar3
void bar3(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[L0:%[0-9]+]] = load i32*, i32** [[DADDR]], align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE:BYREF"(i32* [[L0]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for lastprivate(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar4
void bar4(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[L0:%[0-9]+]] = load i32*, i32** [[DADDR]], align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF"(i32* [[L0]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for reduction(+:d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}
