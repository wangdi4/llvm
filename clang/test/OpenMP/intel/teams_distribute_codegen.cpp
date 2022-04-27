// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fintel-compatibility \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fintel-compatibility -DSPLIT \
// RUN:  -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -std=c++14 -fexceptions \
// RUN:  -fintel-compatibility -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void bar(int,int,...);

// The CodeGen for combined directives should be the same as the
// non-combined directives.

// CHECK-LABEL: foo2
void foo2() {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  int i;
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TEAMS"()
  // CHECK: store i32 0, ptr [[OMP_LB]],
  // CHECK: store i32 15, ptr [[OMP_UB]],
  // CHECK: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.DISTRIBUTE"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB]]
  // CHECK: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, ptr [[OMP_UB]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // CHECK: store i32 {{.*}} ptr [[I]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, ptr [[I]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, ptr [[J]], align 4
  // CHECK-NEXT: {{call|invoke}} void {{.*}}bar
  // CHECK-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

  #pragma omp target
#ifdef SPLIT
  #pragma omp teams
  #pragma omp distribute
#else
  #pragma omp teams distribute
#endif
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // CHECK: directive.region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE"
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
