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

void test_two() {
  int k,j,i,a;
// CHECK: "DIR.OMP.TARGET"
// HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.iv,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.lb,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.ub,
// HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.iv7,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.lb1,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.ub2,
// HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.iv8,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.lb3,
// HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr {{.*}}%.omp.{{.*}}.ub4,
// CHECK: "DIR.OMP.END.TARGET"
#pragma omp metadirective when(user={condition(1)}: target teams distribute parallel for simd collapse(3)) when(user={condition(0)}: target teams distribute parallel for private(i) collapse(2)) default(target teams loop collapse(3))
  for (k = 1; k <= 10; k++) {
    for (j = 1; j <= 10; j++) {
#pragma omp metadirective when(user={condition(0)}: simd) default()
      for (i = 1; i <= 10; i++) {
        a++;
      }
    }
  }
}
