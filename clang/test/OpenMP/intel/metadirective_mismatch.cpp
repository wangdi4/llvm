// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu \
// RUN:   | FileCheck %s --check-prefixes CHECK,HOST

// RUN: %clang_cc1 -emit-llvm-bc -o %t_host.bc %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -triple spir64 -fopenmp-is-device \
// RUN:   -fopenmp-host-ir-file-path %t_host.bc \
// RUN:   | FileCheck %s

// expected-no-diagnostics

// CHECK-LABEL: test_one
void test_one() {

  int A[1024] = {0};

  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"{{.*}}%A
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.iv
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.lb
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.ub
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%i
  // CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target map(tofrom: A)
  {
    // We expect at least one of these when conditons to eval to true, thus having the nothing directive utilized
    #pragma omp metadirective \
       when( device={kind(nohost)}: nothing ) \
       when( device={arch("nvptx")}: nothing) \
       when( implementation={vendor(amd)}: nothing ) \
       default( parallel for)
          for (int i = 0; i < 1024; i++) {
             A[i] += 1;
          }
  }
}
