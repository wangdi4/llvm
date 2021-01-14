// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefixes=TARG,ALL

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fno-intel-openmp-use-llvm-atomic \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host_old.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fno-intel-openmp-use-llvm-atomic \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host_old.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefixes=TARG-OLD,ALL

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.15.26732 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host_win.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -aux-triple x86_64-pc-windows-msvc \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host_win.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefixes=TARG,ALL

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.15.26732 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fno-intel-openmp-use-llvm-atomic \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host_win_old.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -aux-triple x86_64-pc-windows-msvc \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fno-intel-openmp-use-llvm-atomic \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host_win_old.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s --check-prefixes=TARG-OLD-WIN,ALL

#define N 100

//ALL-LABEL: main
int main()
{
  int b = 0;

  //ALL: "DIR.OMP.TARGET"()
  #pragma omp target map(tofrom: b)
  {
    //ALL: "DIR.OMP.PARALLEL.LOOP"()
    #pragma omp parallel for
    for(int ii = 0; ii < N; ++ii)
      //HOST: atomicrmw add {{.*}}1 seq_cst
      //TARG: atomicrmw add {{.*}}1 seq_cst
      //TARG-OLD: call void @__atomic_load
      //TARG-OLD: call {{.*}}@__atomic_compare_exchange
      #pragma omp atomic seq_cst
      b++;
    //ALL: "DIR.OMP.END.PARALLEL.LOOP"()
  }
  //ALL: "DIR.OMP.END.TARGET"()
  return 0;
}

//ALL-LABEL: test_update
void test_update() {
  float counter_N0{};
  //ALL: "DIR.OMP.TARGET"()
  #pragma omp target map(tofrom: counter_N0)
  #pragma omp simd
  for (int i0 = 0 ; i0 < 10 ; i0++ )
  {
    //TARG-OLD-NOT: load atomic
    //TARG-OLD-NOT: cmpxchg
    //TARG: load atomic
    //TARG: cmpxchg
    #pragma omp atomic update
    counter_N0 = counter_N0 +  1. ;
  }
  //ALL: "DIR.OMP.END.TARGET"()
}

//ALL-LABEL: test_write
void test_write(float *x, float *v) {
  #pragma omp target parallel for map(tofrom: x[0], v[0])
  for (int i = 0; i < 100; i++) {
    //TARG-OLD-NOT: store atomic
    //TARG: store atomic
    #pragma omp atomic write
    *x = *v;
  }
}
