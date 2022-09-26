// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s \
// RUN:  --check-prefixes=CHECK,ALL

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline \
// RUN:  -fopenmp-declare-target-scalar-defaultmap-firstprivate \
// RUN:  -fopenmp-declare-target-global-default-map \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s \
// RUN:  --check-prefixes=FPRIVATE,ALL

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-targets=spir64 \
// RUN:  -fopenmp-declare-target-scalar-defaultmap-firstprivate \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s \
// RUN:  --check-prefixes=FPRIVATE-CHECK,ALL

// ALL: [[GLOBSIX:@global_sixteen2]] = target_declare global i32 16

void bar(int,int,...);

#pragma omp declare target
int global_sixteen2 = 16;
#pragma omp end declare target

//CHECK-LABEL: foo2_global2
void foo2_global2() {
  int ii;
  int jj = 20;
// ALL-DAG: [[CE:%.capture_expr.0]] = alloca i32
// ALL-DAG: [[CE1:%.capture_expr.1]] = alloca i32
// ALL-DAG: [[IV:%.omp.iv]] = alloca i32
// ALL-DAG: [[LB:%.omp.lb]] = alloca i32
// ALL-DAG: [[UB:%.omp.ub]] = alloca i32
//CHECK: "DIR.OMP.TARGET"
//CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[GLOBSIX]])
//CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[CE1]])
//CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[IV]])
//CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[LB]])
//CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[UB]])
//CHECK: "DIR.OMP.PARALLEL.LOOP"
//CHECK: "DIR.OMP.END.PARALLEL.LOOP"
//CHECK: "DIR.OMP.END.TARGET"
//FPRIVATE: "DIR.OMP.TARGET"
//FPRIVATE-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[GLOBSIX]])
//FPRIVATE-SAME: "QUAL.OMP.PRIVATE"(ptr [[CE1]])
//FPRIVATE-SAME: "QUAL.OMP.PRIVATE"(ptr [[IV]])
//FPRIVATE-SAME: "QUAL.OMP.PRIVATE"(ptr [[LB]])
//FPRIVATE-SAME: "QUAL.OMP.PRIVATE"(ptr [[UB]])
//FPRIVATE: "DIR.OMP.PARALLEL.LOOP"
//FPRIVATE: "DIR.OMP.END.PARALLEL.LOOP"
//FPRIVATE-CHECK: "DIR.OMP.TARGET"
//FPRIVATE-CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[GLOBSIX]])
//FPRIVATE-CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[IV]])
//FPRIVATE-CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[LB]])
//FPRIVATE-CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[UB]])
//FPRIVATE-CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr [[CE]])
//FPRIVATE-CHECK: "DIR.OMP.PARALLEL.LOOP"
//FPRIVATE-CHECK: "DIR.OMP.END.PARALLEL.LOOP"
//FPRIVATE-CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp parallel for
  for(ii=0;ii<global_sixteen2;++ii) {
    bar(global_sixteen2,ii,jj);
  }
}

// end INTEL_COLLAB
