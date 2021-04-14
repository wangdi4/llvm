// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK: @{{.*}}foobar =
// CHECK: [[FP:@.+]] = internal target_declare global
// CHECK: [[FP1:@.+]] = internal target_declare global
struct A { int i[4]; };
void bar() {
  const A foobar = {1,2,3,4};
  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(%struct.A* %foobar)
  // CHECK-SAME: "QUAL.OMP.MAP.TO:ALWAYS"(%struct.A* [[FP]]
  #pragma omp target firstprivate(foobar)
  {
    // CHECK: [[L:%[0-9]+]] = load i32, i32* getelementptr inbounds (%struct.A, %struct.A* [[FP]]
     int j = foobar.i[0];
  }
  // CHECK: "DIR.OMP.END.TARGET"
}

void foo() {
  const auto lambda = [=]( int i ){ bar(); };
  // CHECK: "DIR.OMP.TARGET"
  // CHECK-SAME: "QUAL.OMP.MAP.TO:ALWAYS"(%class.anon* [[FP1]]
  // CHECK: "DIR.OMP.TEAMS"
  // CHECK-SAME: "QUAL.OMP.SHARED"(%class.anon* [[FP1]]
  #pragma omp target teams
  {
    // CHECK: call void @_ZZ3foovENKUliE_clEi(%class.anon* nonnull dereferenceable(1) [[FP1]]
    lambda(0);
  }
  // CHECK: "DIR.OMP.END.TEAMS"
  // CHECK: "DIR.OMP.END.TARGET"
}

// INTEL_COLLAB
