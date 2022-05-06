// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-version=50 -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// Verify capture expression variables are generated with an integer suffix.

// CHECK-LABEL: @_Z5test1{{.*}}(
int test1() {
  // CHECK: %.capture_expr.0 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.1 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.2 = alloca i8, align 1
  // CHECK-NEXT: %.capture_expr.3 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.4 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.5 = alloca i8, align 1
  // CHECK-NEXT: %.capture_expr.6 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.7 = alloca i32, align 4
  // CHECK-NEXT: %.capture_expr.8 = alloca i64, align 8
  #pragma omp for collapse(2)
  for (int i = 0; i < 4; i++) {
    for (int j = i; j < 4 + i; j++) {
    }
  }
  return 0;
}

int test2(int n) {
  int avar[n];
  // CHECK: %.capture_expr.9 = alloca ptr, align 8
  // CHECK-NEXT: %.capture_expr.10 = alloca ptr, align 8
  // CHECK-NEXT: %.capture_expr.11 = alloca i64, align 8
  #pragma omp teams distribute
  for (auto &aref : avar) {
    aref++;
  }
  return 0;
}
// end INTEL_COLLAB
