// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK: @{{.*}}foobar =
struct A { int i[4]; };
void bar() {
  const A foobar = {1,2,3,4};
  // CHECK: [[FP:%.+]] = alloca %struct.A
  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(%struct.A* %foobar)
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(%struct.A* [[FP]]
  #pragma omp target firstprivate(foobar)
  {
     int j = foobar.i[0];
  }
  // CHECK: "DIR.OMP.END.TARGET"
}

void foo() {
  const auto lambda = [=]( int i ){ bar(); };
  // CHECK: [[FP1:%.+]] = alloca %class.anon
  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(%class.anon* [[FP1]]
  // CHECK: "DIR.OMP.TEAMS"
  // CHECK-SAME: "QUAL.OMP.SHARED"(%class.anon* [[FP1]]
  #pragma omp target teams
  {
    // CHECK: call void @_ZZ3foovENKUliE_clEi(%class.anon* noundef {{.*}} [[FP1]]
    lambda(0);
  }
  // CHECK: "DIR.OMP.END.TEAMS"
  // CHECK: "DIR.OMP.END.TARGET"
}

// INTEL_COLLAB
