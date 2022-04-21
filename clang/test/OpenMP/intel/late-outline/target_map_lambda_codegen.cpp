// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct SS {
  int a;
  int b;
  SS();
};
SS::SS() {
    this->a = 0; this->b=0;
    auto lambda1 = [&]() {++this->a;};
    // CHECK: [[L0:%[0-9]+]] = getelementptr inbounds %class.anon, ptr %lambda1, i32 0, i32 0
    // CHECK: [[L:%[0-9]+]] = getelementptr inbounds %class.anon, ptr %lambda1, i32 0, i32 0
    // CHECK: [[L1:%[0-9]+]] = getelementptr inbounds %class.anon, ptr %lambda1, i32 0, i32 0
    // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
    // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %this1, ptr %this1, i64 8, i64 547,
    // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %lambda1, ptr %lambda1, i64 8, i64 673
    // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L]], ptr [[L1]], i64 8, i64 562949953422096,
    #pragma omp target
    lambda1();
    // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  }
void foo() {
  SS ss;
  #pragma omp target enter data map(alloc:ss.a, ss.b)
  auto lambda = [&]( ){ ss.a++; ss.b++; };
  // CHECK: [[L0:%[0-9]+]] = getelementptr inbounds %class.anon.0, ptr %lambda, i32 0, i32 0
  // CHECK: [[L:%[0-9]+]] = getelementptr inbounds %class.anon.0, ptr %lambda, i32 0, i32 0
  // CHECK: [[L1:%[0-9]+]] = getelementptr inbounds %class.anon.0, ptr %lambda, i32 0, i32 0
  // CHECK: [[L2:%[0-9]+]] = load ptr, ptr [[L1]]
  // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %ss, ptr %ss, i64 8, i64 547,
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %lambda, ptr %lambda, i64 8, i64 673
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[L]], ptr [[L2]], i64 8, i64 562949953422096,
  #pragma omp target
    lambda();
  // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}
int main()
{
  foo();
}
// end INTEL_COLLAB
