// RUN: %clang -c -S -emit-llvm -fhonor-nan-compares -ffast-math -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NANCMP
// RUN: %clang -c -S -emit-llvm -fno-honor-nan-compares -ffast-math -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP
// RUN: %clang -c -S -emit-llvm -ffast-math -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP
// RUN: %clang -c -S -emit-llvm -fhonor-nan-compares -ffp-model=fast -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NANCMP
// RUN: %clang -c -S -emit-llvm -fno-honor-nan-compares -ffp-model=fast -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP
// RUN: %clang -c -S -emit-llvm -ffp-model=fast -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP
// RUN: %clang -c -S -emit-llvm -fhonor-nan-compares -ffinite-math-only -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-FIN-NANCMP
// RUN: %clang -c -S -emit-llvm -fno-honor-nan-compares -ffinite-math-only -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-FIN-NONANCMP
// RUN: %clang -c -S -emit-llvm -ffinite-math-only -target x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-FIN-NONANCMP
// RUN: %clang_cc1 -emit-llvm -fhonor-nan-compares -ffast-math -ffp-contract=fast -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NANCMP
// RUN: %clang_cc1 -emit-llvm -fno-honor-nan-compares -ffast-math -ffp-contract=fast -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP
// RUN: %clang_cc1 -emit-llvm -ffast-math -ffp-contract=fast -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONANCMP

// CHECK: define {{.*}} @test_self_eq_f32
int test_self_eq_f32(float x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq float
// CHECK-NONANCMP: fcmp fast oeq float
// CHECK-FIN-NANCMP: fcmp ninf oeq float
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq float
  return x == x;
}

// CHECK: define {{.*}} @test_self_eq_f64
int test_self_eq_f64(double x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq double
// CHECK-NONANCMP: fcmp fast oeq double
// CHECK-FIN-NANCMP: fcmp ninf oeq double
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq double
  return x == x;
}

// CHECK: define {{.*}} @test_self_ne_f32
int test_self_ne_f32(float x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn une float
// CHECK-NONANCMP: fcmp fast une float
// CHECK-FIN-NANCMP: fcmp ninf une float
// CHECK-FIN-NONANCMP: fcmp nnan ninf une float
  return x != x;
}

// CHECK: define {{.*}} @test_self_ne_f64
int test_self_ne_f64(double x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn une double
// CHECK-NONANCMP: fcmp fast une double
// CHECK-FIN-NANCMP: fcmp ninf une double
// CHECK-FIN-NONANCMP: fcmp nnan ninf une double
  return x != x;
}

// CHECK: define {{.*}} @test_nan_eq_f32
int test_nan_eq_f32(float x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq float
// CHECK-NONANCMP: fcmp fast oeq float
// CHECK-FIN-NANCMP: fcmp ninf oeq float
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq float
  return x == __builtin_nanf("");
}

// CHECK: define {{.*}} @test_nan_eq_f64
int test_nan_eq_f64(double x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq double
// CHECK-NONANCMP: fcmp fast oeq double
// CHECK-FIN-NANCMP: fcmp ninf oeq double
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq double
  return x == __builtin_nan("");
}

// CHECK: define {{.*}} @test_nan_ne_f32
int test_nan_ne_f32(float x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn une float
// CHECK-NONANCMP: fcmp fast une float
// CHECK-FIN-NANCMP: fcmp ninf une float
// CHECK-FIN-NONANCMP: fcmp nnan ninf une float
  return x != __builtin_nanf("");
}

// CHECK: define {{.*}} @test_nan_ne_f64
int test_nan_ne_f64(double x) {
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn une double
// CHECK-NONANCMP: fcmp fast une double
// CHECK-FIN-NANCMP: fcmp ninf une double
// CHECK-FIN-NONANCMP: fcmp nnan ninf une double
  return x != __builtin_nan("");
}

typedef float v4f32 __attribute__((vector_size(16)));
typedef int   v4i32 __attribute__((vector_size(16)));

// CHECK: define {{.*}} @test_vec_eq_f32
v4i32 test_vec_eq_f32(v4f32 x, v4f32 y) {
  return x == y;
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq <4 x float>
// CHECK-NONANCMP: fcmp fast oeq <4 x float>
// CHECK-FIN-NANCMP: fcmp ninf oeq <4 x float>
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq <4 x float>
}

// CHECK: define {{.*}} @test_vec_gt_f32
v4i32 test_vec_gt_f32(v4f32 x, v4f32 y) {
  return x > y;
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn ogt <4 x float>
// CHECK-NONANCMP: fcmp fast ogt <4 x float>
// CHECK-FIN-NANCMP: fcmp ninf ogt <4 x float>
// CHECK-FIN-NONANCMP: fcmp nnan ninf ogt <4 x float>
}

typedef double v2f64 __attribute__((vector_size(16)));
typedef long   v2i64 __attribute__((vector_size(16)));

// CHECK: define {{.*}} @test_vec_eq_f64
v2i64 test_vec_eq_f64(v2f64 x, v2f64 y) {
  return x == y;
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq <2 x double>
// CHECK-NONANCMP: fcmp fast oeq <2 x double>
// CHECK-FIN-NANCMP: fcmp ninf oeq <2 x double>
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq <2 x double>
}

// CHECK: define {{.*}} @test_vec_gt_f64
v2i64 test_vec_gt_f64(v2f64 x, v2f64 y) {
  return x > y;
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn ogt <2 x double>
// CHECK-NONANCMP: fcmp fast ogt <2 x double>
// CHECK-FIN-NANCMP: fcmp ninf ogt <2 x double>
// CHECK-FIN-NONANCMP: fcmp nnan ninf ogt <2 x double>
}

// CHECK: define {{.*}} @test_const_nan_eq_f64
const double g3 = 3.0;
int test_const_nan_eq_f64(double x) {
// CHECK-NOT: fcmp
// CHECK: ret i32 0
  return g3 == __builtin_nan("");
}

// CHECK: define {{.*}} @test_self_sub_eq0_f64
int test_self_sub_eq0_f64(double x) {
  double t = x - x;
  return t == 0;
// CHECK-NANCMP: fsub reassoc ninf nsz arcp contract afn double
// CHECK-NANCMP: fcmp reassoc ninf nsz arcp contract afn oeq double
// CHECK-NONANCMP: fsub fast double
// CHECK-NONANCMP: fcmp fast oeq double
// CHECK-FIN-NANCMP: fsub ninf double
// CHECK-FIN-NANCMP: fcmp ninf oeq double
// CHECK-FIN-NONANCMP: fsub nnan ninf double
// CHECK-FIN-NONANCMP: fcmp nnan ninf oeq double
}
