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

// CHECK-LABEL: foo1
void foo1() {
  // CHECK: [[J:%j.*]] = alloca i32,
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL"()
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[J]]
#ifdef SPLIT
  #pragma omp target
  #pragma omp parallel
#else
  #pragma omp target parallel
#endif
  {
    // CHECK: [[L1:%[0-9]+]] = load i32, ptr [[J]], align 4
    // CHECK-NEXT: {{call|invoke}} void {{.*}}bar
    // CHECK-SAME: (i32 noundef 41, i32 noundef [[L1]])
    bar(41,j);
  }
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
