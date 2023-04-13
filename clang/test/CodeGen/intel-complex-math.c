// Test with -fhonor-nan-compares -ffp-contract=fast
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 \
// RUN:-fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple powerpc-unknown-unknown \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabi \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabihf \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple thumbv7k-apple-watchos2.0 \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple aarch64-unknown-unknown \
// RUN: -fhonor-nan-compares -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir -fhonor-nan-compares \
// RUN: -ffp-contract=fast -o - | FileCheck %s

float _Complex mul_float_cc(float _Complex a, float _Complex b) {
  // CHECK-LABEL: @mul_float_cc(
  // CHECK-NOT: call {{.*}} @__divsc3(
  return a * b;
}

float _Complex div_float_cc(float _Complex a, float _Complex b) {
  // CHECK-LABEL: @div_float_cc(
  // CHECK-NOT: call {{.*}} @__divsc3(
  return a / b;
}

double _Complex mul_double_cc(double _Complex a, double _Complex b) {
  // CHECK-LABEL: @mul_double_cc(
  // CHECK-NOT: call {{.*}} @__muldc3(
  return a * b;
}

double _Complex div_double_cc(double _Complex a, double _Complex b) {
  // CHECK-LABEL: @div_double_cc(
  // CHECK-NOT: call {{.*}} @__divdc3(
  return a / b;
}

double _Complex div_double_rc(double a, double _Complex b) {
  // CHECK-LABEL: @div_double_rc(
  // CHECK-NOT: call {{.*}} @__divdc3(
  return a / b;
}

long double _Complex div_long_double_rc(long double a, long double _Complex b) {
  // CHECK-LABEL: @div_long_double_rc(
  // CHECK-NOT: call {{.*}} @__divxc3(
  return a / b;
}

long double _Complex mul_long_double_cc(long double _Complex a, long double _Complex b) {
  // CHECK-LABEL: @mul_long_double_cc(
  // CHECK-NOT: call {{.*}} @__muldc3(
  return a * b;
}

long double _Complex div_long_double_cc(long double _Complex a, long double _Complex b) {
  // CHECK-LABEL: @div_long_double_cc(
  // CHECK-NOT: call {{.*}} @__divxc3(
  return a / b;
}

_Complex double foo(_Complex double a, _Complex double b) {
  // CHECK-LABEL: @foo(
  // CHECK-NOT: call void @__muldc3
  return a * b;
}

