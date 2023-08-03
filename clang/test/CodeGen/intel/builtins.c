// RUN: %clang_cc1 -emit-llvm -o %t %s
// RUN: not grep __builtin %t
// RUN: %clang_cc1 %s -emit-llvm -o - -triple x86_64-darwin-apple | FileCheck %s


// CHECK-LABEL: define{{.*}}void @test_float_builtins
void test_float_builtins(float F, double D, long double LD) {
  volatile int res;
  res = __builtin_isinff(F);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f32(float {{.*}}, i32 504)
  // CHECK: zext i1 [[TMP]] to i32

  res = __builtin_isinf(D);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f64(double {{.*}}, i32 516)
  // CHECK: zext i1 [[TMP]] to i32

  res = __builtin_isinfl(LD);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f80(x86_fp80 {{.*}}, i32 504)
  // CHECK: zext i1 [[TMP]] to i32
  res = __builtin_finite(D);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f64(double {{.*}}, i32 504)
  // CHECK: zext i1 [[TMP]] to i32
  res = __builtin_finitef(F);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f32(float %12, i32 504)
  // CHECK: zext i1 [[TMP]] to i32

  res = __builtin_finitel(LD);
  // CHECK: [[TMP:%.*]] = call i1 @llvm.is.fpclass.f80(x86_fp80 {{.*}}, i32 504)
  // CHECK: zext i1 %16 to i32
}
