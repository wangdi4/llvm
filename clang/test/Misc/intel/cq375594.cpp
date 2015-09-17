// RUN: %clang_cc1 -fintel-compatibility -ast-dump %s | FileCheck %s
// REQUIRES: llvm-backend


__declspec(avoid_false_share) int i;

struct A {
  int i __attribute__((bnd_variable_size));
} __attribute__((gcc_struct));

int f(int x) __attribute__((bnd_legacy));

// CHECK-NOT: AvoidFalseShareAttr
// CHECK-NOT: GCCStructAttr
// CHECK-NOT: BNDVarSizeAttr
// CHECK-NOT: BNDLegacyAttr
