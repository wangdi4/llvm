// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// For C, when argument to an 'if' or 'final' clause is a variable of type
// double, ensure it's converted to a boolean expression (d != 0.0).

int main() {
  double d1, d2;

  // CHECK: [[L1:%[0-9]+]] = load double, ptr %d1
  // CHECK: [[BOOL1:%tobool[0-9]*]] = fcmp une double [[L1]], 0.000000e+00
  // CHECK: [[L2:%[0-9]+]] = load double, ptr %d2
  // CHECK: [[BOOL2:%tobool[0-9]*]] = fcmp une double [[L2]], 0.000000e+00
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.FINAL"(i1 [[BOOL1]])
  // CHECK-SAME: "QUAL.OMP.IF"(i1 [[BOOL2]]) ]
  // CHECK: DIR.OMP.END.TASK
  #pragma omp task final(d1) if(d2)
  {}
}
