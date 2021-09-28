// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown -o - | FileCheck %s --check-prefix=NOINTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 -o - | FileCheck %s --check-prefix=NOINTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple i686-unknown-unknown -o - | FileCheck %s --check-prefix=NOINTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple spir -o - | FileCheck %s --check-prefix=NOINTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown -fuse-complex-intrinsics -o - | FileCheck %s --check-prefix=INTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-pc-win64 -fuse-complex-intrinsics -o - | FileCheck %s --check-prefix=INTRIN
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown -fuse-complex-intrinsics -DT=int -o - | FileCheck %s --check-prefix=INT
// RUN: %clang_cc1 %s -O0 -emit-llvm -triple x86_64-unknown-unknown -DT=int -o - | FileCheck %s --check-prefix=INT

#ifndef T
#  define T float
#endif

T check_var;
// INTRIN: @check_var = global [[T:[a-z0-9]+]]
// NOINTRIN: @check_var = global [[T:[a-z0-9]+]]
// INT: @check_var = global [[T:i[0-9]+]]

T _Complex add_rc(T a, T _Complex b) {
  // INTRIN-LABEL: @add_rc(
  // INTRIN-COUNT-1: fadd [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @add_rc(
  // NOINTRIN-COUNT-1: fadd [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @add_rc(
  // INT-COUNT-1: add [[T]]
  // INT: ret
  return a + b;
}

T _Complex add_cr(T _Complex a, T b) {
  // INTRIN-LABEL: @add_cr(
  // INTRIN-COUNT-1: fadd [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @add_cr(
  // NOINTRIN-COUNT-1: fadd [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @add_cr(
  // INT-COUNT-1: add [[T]]
  // INT: ret
  return a + b;
}

T _Complex add_cc(T _Complex a, T _Complex b) {
  // INTRIN-LABEL: @add_cc(
  // INTRIN-COUNT-2: fadd [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @add_cc(
  // NOINTRIN-COUNT-2: fadd [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @add_cc(
  // INT-COUNT-2: add [[T]]
  // INT: ret
  return a + b;
}

T _Complex sub_rc(T a, T _Complex b) {
  // INTRIN-LABEL: @sub_rc(
  // INTRIN: fsub [[T]]
  // INTRIN: fneg [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @sub_rc(
  // NOINTRIN: fsub [[T]]
  // NOINTRIN: fneg [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @sub_rc(
  // INT-COUNT-2: sub [[T]]
  // INT: ret
  return a - b;
}

T _Complex sub_cr(T _Complex a, T b) {
  // INTRIN-LABEL: @sub_cr(
  // INTRIN: fsub [[T]]
  // INTRIN-NOT: fsub [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @sub_cr(
  // NOINTRIN: fsub [[T]]
  // NOINTRIN-NOT: fsub [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @sub_cr(
  // INT-COUNT-2: sub [[T]]
  // INT: ret
  return a - b;
}

T _Complex sub_cc(T _Complex a, T _Complex b) {
  // INTRIN-LABEL: @sub_cc(
  // INTRIN-COUNT-2: fsub [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @sub_cc(
  // NOINTRIN-COUNT-2: fsub [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @sub_cc(
  // INT-COUNT-2: sub [[T]]
  // INT: ret
  return a - b;
}

T _Complex mul_rc(T a, T _Complex b) {
  // INTRIN-LABEL: @mul_rc(
  // INTRIN-COUNT-2: fmul [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @mul_rc(
  // NOINTRIN-COUNT-2: fmul [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @mul_rc(
  // INT-COUNT-4: mul [[T]]
  // INT: ret
  return a * b;
}

T _Complex mul_cr(T _Complex a, T b) {
  // INTRIN-LABEL: @mul_cr(
  // INTRIN-COUNT-2: fmul [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @mul_cr(
  // NOINTRIN-COUNT-2: fmul [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @mul_cr(
  // INT-COUNT-4: mul [[T]]
  // INT: ret
  return a * b;
}

T _Complex mul_cc(T _Complex a, T _Complex b) {
  // INTRIN-LABEL: @mul_cc(
  // INTRIN-NOT: fmul [[T]]
  // INTRIN: call {{.*}} @llvm.intel.complex.fmul
  // INTRIN: ret
  // NOINTRIN-LABEL: @mul_cc(
  // NOINTRIN-COUNT-4: fmul [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @mul_cc(
  // INT-COUNT-4: mul [[T]]
  // INT: ret
  return a * b;
}

T _Complex div_rc(T a, T _Complex b) {
  // INTRIN-LABEL: @div_rc(
  // INTRIN-NOT: fdiv [[T]]
  // INTRIN: call {{.*}} @llvm.intel.complex.fdiv
  // INTRIN: ret
  // NOINTRIN-LABEL: @div_rc(
  // NOINTRIN: call {{.*}} @__div
  // NOINTRIN: ret
  // INT-LABEL: @div_rc(
  // INT-COUNT-6: mul [[T]]
  // INT: ret
  return a / b;
}

T _Complex div_cr(T _Complex a, T b) {
  // INTRIN-LABEL: @div_cr(
  // INTRIN-COUNT-2: fdiv [[T]]
  // INTRIN: ret
  // NOINTRIN-LABEL: @div_cr(
  // NOINTRIN-COUNT-2: fdiv [[T]]
  // NOINTRIN: ret
  // INT-LABEL: @div_cr(
  // INT-COUNT-5: mul [[T]]
  // INT: ret
  return a / b;
}

T _Complex div_cc(T _Complex a, T _Complex b) {
  // INTRIN-LABEL: @div_cc(
  // INTRIN-NOT: fdiv [[T]]
  // INTRIN: call {{.*}} @llvm.intel.complex.fdiv
  // INTRIN: ret
  // NOINTRIN-LABEL: @div_cc(
  // NOINTRIN: call {{.*}} @__div
  // NOINTRIN: ret
  // INT-LABEL: @div_cc(
  // INT-COUNT-6: mul [[T]]
  // INT: ret
  return a / b;
}
