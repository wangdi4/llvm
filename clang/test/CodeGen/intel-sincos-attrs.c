// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -ffast-math -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-APPROX %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fapprox-func -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-APPROX %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:low -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-LOWPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:medium -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-MEDIUMPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:high -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-HIGHPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:low -fapprox-func -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-LOWPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:medium -fapprox-func -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-MEDIUMPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -mGLOB_imf_attr=precision:high -fapprox-func -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECOVERRIDE,CHECK-HIGHPREC %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECISE,CHECK-PRECISE-NOERRNO %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fmath-errno -emit-llvm -o - %s | FileCheck -check-prefixes CHECK,CHECK-PRECISE,CHECK-PRECISE-ERRNO %s

void sincosf(float, float *, float *);
void sincos(double, double *, double *);

// CHECK: define {{.*}} float @sincos_single_prec(
// CHECK-APPROX: call void @sincosf({{.*}} #[[ATTR_APPROX:[0-9]+]]
// CHECK-PRECISE-NOT: call void @sincosf({{.*}} #{{.*}}
// CHECK-PRECOVERRIDE: call void @sincosf({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
float sincos_single_prec(float a) {
  float sin = 0.f, cos = 0.f;
  sincosf(a, &sin, &cos);
  return sin;
}

// CHECK: define {{.*}} double @sincos_double_prec(
// CHECK-APPROX: call void @sincos({{.*}} #[[ATTR_APPROX:[0-9]+]]
// CHECK-PRECISE-NOT: call void @sincos({{.*}} #{{.*}}
// CHECK-PRECOVERRIDE: call void @sincos({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
double sincos_double_prec(double a) {
  double sin = 0.0, cos = 0.0;
  sincos(a, &sin, &cos);
  return sin;
}

float sinf(float);
double sin(double);

// CHECK: define {{.*}} float @sin_single_prec(
// CHECK-APPROX: call{{.*}} afn{{.*}} float @llvm.sin.f32(
// CHECK-PRECISE-NOERRNO: call float @llvm.sin.f32(
// CHECK-PRECISE-ERRNO: call float @sinf(
// CHECK-PRECOVERRIDE: call{{.*}} float @llvm.sin.f32({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
float sin_single_prec(float a) {
  return sinf(a);
}

// CHECK: define {{.*}} double @sin_double_prec(
// CHECK-APPROX: call{{.*}} afn{{.*}} double @llvm.sin.f64(
// CHECK-PRECISE-NOERRNO: call double @llvm.sin.f64(
// CHECK-PRECISE-ERRNO: call double @sin(
// CHECK-PRECOVERRIDE: call{{.*}} double @llvm.sin.f64({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
double sin_double_prec(double a) {
  return sin(a);
}

void randomfuncf(float, float *, float *);
void randomfunc(double, double *, double *);

// CHECK: define{{.*}} float @randomfunc_single_prec(
// CHECK-APPROX-NOT: call void @randomfuncf({{.*}} #{{.*}}
// CHECK-PRECISE-NOT: call void @randomfuncf({{.*}} #{{.*}}
// CHECK-PRECOVERRIDE: call void @randomfuncf({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
float randomfunc_single_prec(float a) {
  float b = 0.f, c = 0.f;
  randomfuncf(a, &b, &c);
  return b;
}

// CHECK: define{{.*}} double @randomfunc_double_prec(
// CHECK-APPROX-NOT: call void @randomfunc({{.*}} #{{.*}}
// CHECK-PRECISE-NOT: call void @randomfunc({{.*}} #{{.*}}
// CHECK-PRECOVERRIDE: call void @randomfunc({{.*}} #[[ATTR_PRECOVERRIDE:[0-9]+]]
double randomfunc_double_prec(double a) {
  double b = 0.0, c = 0.0;
  randomfunc(a, &b, &c);
  return b;
}

// CHECK-APPROX: attributes #[[ATTR_APPROX]] = { "imf-precision"="medium" }
// CHECK-LOWPREC: attributes #[[ATTR_PRECOVERRIDE]] = { {{.*}}"imf-precision"="low"{{.*}} }
// CHECK-MEDIUMPREC: attributes #[[ATTR_PRECOVERRIDE]] = { {{.*}}"imf-precision"="medium"{{.*}} }
// CHECK-HIGHPREC: attributes #[[ATTR_PRECOVERRIDE]] = { {{.*}}"imf-precision"="high"{{.*}} }
