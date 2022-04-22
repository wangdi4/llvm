// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct SS {
  int a;
  int b;
  SS();
};
SS::SS() {
    this->a = 0; this->b=0;
    auto lambda1 = [&]() {++this->a;};
    // CHECK: [[L0:%[0-9]+]] = getelementptr inbounds %class.anon, %class.anon* %lambda1, i32 0, i32 0
    // CHECK: [[L:%[0-9]+]] = getelementptr inbounds %class.anon, %class.anon* %lambda1, i32 0, i32 0
    // CHECK: [[L1:%[0-9]+]] = getelementptr inbounds %class.anon, %class.anon* %lambda1, i32 0, i32 0
    // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
    // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.SS* %this1, %struct.SS* %this1, i64 8, i64 547,
    // CHECK-SAME: "QUAL.OMP.MAP.TO"(%class.anon* %lambda1, %class.anon* %lambda1, i64 8, i64 673
    // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SS** [[L]], %struct.SS** [[L1]], i64 8, i64 562949953422096,
    #pragma omp target
    lambda1();
    // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  }
void foo() {
  SS ss;
  #pragma omp target enter data map(alloc:ss.a, ss.b)
  auto lambda = [&]( ){ ss.a++; ss.b++; };
  // CHECK: [[L0:%[0-9]+]] = getelementptr inbounds %class.anon.0, %class.anon.0* %lambda, i32 0, i32 0
  // CHECK: [[L:%[0-9]+]] = getelementptr inbounds %class.anon.0, %class.anon.0* %lambda, i32 0, i32 0
  // CHECK: [[L1:%[0-9]+]] = getelementptr inbounds %class.anon.0, %class.anon.0* %lambda, i32 0, i32 0
  // CHECK: [[L2:%[0-9]+]] = load %struct.SS*, %struct.SS** %10
  // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.SS* %ss, %struct.SS* %ss, i64 8, i64 547,
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(%class.anon.0* %lambda, %class.anon.0* %lambda, i64 8, i64 673
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.SS** [[L]], %struct.SS* [[L2]], i64 8, i64 562949953422096,
  #pragma omp target
    lambda();
  // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
}
int main()
{
  foo();
}
// end INTEL_COLLAB
