// RUN: %clang_cc1 -emit-llvm -o %t %s
// RUN: not grep __builtin %t
// RUN: %clang_cc1 %s -emit-llvm -o - -triple x86_64-darwin-apple | FileCheck %s


// CHECK-LABEL: define{{.*}}void @test_float_builtins
void test_float_builtins(float F, double D, long double LD) {
  volatile int res;
  res = __builtin_isinff(F);
  // CHECK:  call float @llvm.fabs.f32(float
  // CHECK:  fcmp oeq float {{.*}}, 0x7FF0000000000000

  res = __builtin_isinf(D);
  // CHECK:  call double @llvm.fabs.f64(double
  // CHECK:  fcmp oeq double {{.*}}, 0x7FF0000000000000

  res = __builtin_isinfl(LD);
  // CHECK:  call x86_fp80 @llvm.fabs.f80(x86_fp80
  // CHECK:  fcmp oeq x86_fp80 {{.*}}, 0xK7FFF8000000000000000
  res = __builtin_finite(D);
  // CHECK: call double @llvm.fabs.f64(double
  // CHECK: fcmp one double {{.*}}, 0x7FF0000000000000
  res = __builtin_finitef(F);
  // CHECK: call float @llvm.fabs.f32(float
  // CHECK: fcmp one float {{.*}}, 0x7FF0000000000000

  res = __builtin_finitel(LD);
  // CHECK:  call x86_fp80 @llvm.fabs.f80(x86_fp80
  // CHECK:  fcmp one x86_fp80 {{.*}}, 0xK7FFF8000000000000000
}
