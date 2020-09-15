// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -fintel-openmp-use-llvm-atomic \
// RUN:  -emit-llvm %s -o - | FileCheck %s --check-prefix=ALL

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -fintel-openmp-use-llvm-atomic \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fintel-openmp-use-llvm-atomic \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefix=ALL

#define N 100

int main()
{
  int b = 0;

  //ALL: "DIR.OMP.TARGET"()
  #pragma omp target map(tofrom: b)
  {
    //ALL: "DIR.OMP.PARALLEL.LOOP"()
    #pragma omp parallel for
    for(int ii = 0; ii < N; ++ii)
      //ALL: atomicrmw add {{.*}}1 seq_cst
      #pragma omp atomic seq_cst
      b++;
    //ALL: "DIR.OMP.END.PARALLEL.LOOP"()
  }
  //ALL: "DIR.OMP.END.TARGET"()
  return 0;
}
