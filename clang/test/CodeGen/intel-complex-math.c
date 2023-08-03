// Test with -fhonor-nan-compares
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-windows-msvc \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=WIN

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple powerpc-unknown-unknown \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-PPC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabi \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabihf \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple thumbv7k-apple-watchos2.0 \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple aarch64-unknown-unknown \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir -fhonor-nan-compares -fintel-compatibility   \
// RUN: -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown \
// RUN: -fhonor-nan-compares -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple powerpc-unknown-unknown \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabi \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabihf \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple thumbv7k-apple-watchos2.0 \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple aarch64-unknown-unknown \
// RUN: -fhonor-nan-compares -ffreestanding -fintel-compatibility  -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir -fhonor-nan-compares \
// RUN:  -ffreestanding -fintel-compatibility  -ffp-contract=fast -o - | FileCheck %s  

// Test with -fhonor-nans
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-windows-msvc \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=WIN

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple powerpc-unknown-unknown \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-PPC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabi \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabihf \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple thumbv7k-apple-watchos2.0 \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple aarch64-unknown-unknown \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir  -fintel-compatibility   \
// RUN: -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC-ARM

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown \
// RUN:  -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s --check-prefix=INTRINSIC

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple powerpc-unknown-unknown \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabi \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple armv7-none-linux-gnueabihf \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple thumbv7k-apple-watchos2.0 \
// RUN:  -ffreestanding -fintel-compatibility   -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple aarch64-unknown-unknown \
// RUN:  -ffreestanding -fintel-compatibility  -ffp-contract=fast -o - | FileCheck %s

// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir  \
// RUN:  -ffreestanding -fintel-compatibility  -ffp-contract=fast -o - | FileCheck %s

float _Complex mul_float_cc(float _Complex a, float _Complex b) {
  // CHECK-LABEL: @mul_float_cc(
  // WIN-NOT: call {{.*}} @__mulsc3(
  // INTRINSIC-NOT: call {{.*}} @__mulsc3(
  // INTRINSIC-PPC-NOT: call {{.*}} @__mulsc3(
  // CHECK-NOT: call {{.*}} @__mulsc3(
  return a * b;
}

float _Complex div_float_cc(float _Complex a, float _Complex b) {
  // CHECK-LABEL: @div_float_cc(
  // WIN-NOT: call {{.*}} @__divsc3(
  // INTRINSIC: call {{.*}} @__divsc3(
  // INTRINSIC-PPC: call {{.*}} @__divsc3(
  // CHECK-NOT: call {{.*}} @__divsc3(
  return a / b;
}

double _Complex mul_double_cc(double _Complex a, double _Complex b) {
  // CHECK-LABEL: @mul_double_cc(
  // WIN-NOT: call {{.*}} @__muldc3(
  // INTRINSIC-NOT: call {{.*}} @__muldc3(
  // INTRINSIC-PPC-NOT: call {{.*}} @__muldc3(
  // CHECK-NOT: call {{.*}} @__muldc3(
  return a * b;
}

double _Complex div_double_cc(double _Complex a, double _Complex b) {
  // CHECK-LABEL: @div_double_cc(
  // WIN-NOT: call {{.*}} @__divdc3(
  // INTRINSIC: call {{.*}} @__divdc3(
  // INTRINSIC-PPC: call {{.*}} @__divdc3(
  // CHECK-NOT: call {{.*}} @__divdc3(
  return a / b;
}

double _Complex div_double_rc(double a, double _Complex b) {
  // CHECK-LABEL: @div_double_rc(
  // WIN-NOT: call {{.*}} @__divdc3(
  // INTRINSIC: call {{.*}} @__divdc3(
  // INTRINSIC-PPC: call {{.*}} @__divdc3(
  // CHECK-NOT: call {{.*}} @__divdc3(
  return a / b;
}

long double _Complex div_long_double_rc(long double a, long double _Complex b) {
  // CHECK-LABEL: @div_long_double_rc(
  // WIN-NOT: call {{.*}} @__divxc3(
  // INTRINSIC-NOT: call {{.*}} @__divxc3(
  // INTRINSIC-PPC: call {{.*}} @__divtc3(
  // CHECK-NOT: call {{.*}} @__divxc3(
  return a / b;
}

long double _Complex mul_long_double_cc(long double _Complex a, long double _Complex b) {
  // CHECK-LABEL: @mul_long_double_cc(
  // WIN-NOT: call {{.*}} @__muldc3(
  // INTRINSIC-NOT: call {{.*}} @__muldc3(
  // INTRINSIC-PPC-NOT: call {{.*}} @__muldc3(
  // CHECK-NOT: call {{.*}} @__muldc3(
  return a * b;
}

long double _Complex div_long_double_cc(long double _Complex a, long double _Complex b) {
  // CHECK-LABEL: @div_long_double_cc(
  // WIN-NOT: call {{.*}} @__divxc3(
  // INTRINSIC: call {{.*}} @__divxc3(
  // INTRINSIC-PPC: call {{.*}} @__divtc3(
  // INTRINSIC-ARM: call {{.*}} @__divdc3(
  // CHECK-NOT: call {{.*}} @__divxc3(
  return a / b;
}
