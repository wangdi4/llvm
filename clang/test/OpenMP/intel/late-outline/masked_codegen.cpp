// INTEL_COLLAB

//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline -DUSE_MASTER \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fopenmp-late-outline -fopenmp-version=51 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -triple spir64 -fopenmp -fopenmp-late-outline \
// RUN:  -fopenmp-version=51 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc -emit-llvm -o - %s \
// RUN:  | FileCheck %s --check-prefix TARG

#ifdef USE_MASTER
#define PRAGMA master
#else
#define PRAGMA masked
#endif

void foo() { extern void mayThrow(); mayThrow(); }

//CHECK-LABEL: main
int main() {
  char a;

  //CHECK: "DIR.OMP.MASTER"
  #pragma omp PRAGMA
  a = 2;

  //CHECK: "DIR.OMP.END.MASTER"
  //CHECK: "DIR.OMP.MASTER"
  #pragma omp PRAGMA
  foo();
  //CHECK: "DIR.OMP.END.MASTER"

  //CHECK: "DIR.OMP.MASTER"
  #pragma omp masked filter(a)
  foo();
  //CHECK: "DIR.OMP.END.MASTER"

  //CHECK: "DIR.OMP.TARGET"
  //TARG: "DIR.OMP.TARGET"
  #pragma omp target
  {
    //CHECK: "DIR.OMP.MASTER"
    //TARG: "DIR.OMP.MASTER"
    #pragma omp masked
    //CHECK: "DIR.OMP.END.MASTER"
    //TARG: "DIR.OMP.END.MASTER"
    foo();
  }
  //CHECK: "DIR.OMP.END.TARGET"
  //TARG: "DIR.OMP.END.TARGET"
  return a;
}

//CHECK-LABEL: parallel_masked
void parallel_masked() {
//CHECK: "DIR.OMP.PARALLEL"
#pragma omp parallel
//CHECK: "DIR.OMP.MASTER"
#pragma omp PRAGMA
  foo();
//CHECK: "DIR.OMP.END.MASTER"
//CHECK: "DIR.OMP.END.PARALLEL"
}

// end INTEL_COLLAB
