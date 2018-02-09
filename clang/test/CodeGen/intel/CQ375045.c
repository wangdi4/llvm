// REQUIRES: llvm-backend
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple i686-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

// CHECK-LABEL: @f1
int f1(int v, char c) {
  // CHECK: [[RHS:%.+]] = sext i8
  // CHECK: [[AND:%.+]] = and i{{16|32}} [[RHS]], {{15|31}}
  // CHECK: [[RES:%.+]] = shl i{{16|32}} %{{.+}}, [[AND]]
  // CHECK: ret i{{16|32}} [[RES]]
  return v << c;
}

// CHECK-LABEL: @f2
int f2(int v, char c) {
  // CHECK: [[RHS:%.+]] = sext i8
  // CHECK: [[AND:%.+]] = and i{{16|32}} [[RHS]], {{15|31}}
  // CHECK: [[RES:%.+]] = ashr i{{16|32}} %{{.+}}, [[AND]]
  // CHECK: ret i{{16|32}} [[RES]]
  return v >> c;
}

// CHECK-LABEL: @f3
unsigned int f3(unsigned int v, char c) {
  // CHECK: [[RHS:%.+]] = sext i8
  // CHECK: [[AND:%.+]] = and i{{16|32}} [[RHS]], {{15|31}}
  // CHECK: [[RES:%.+]] = lshr i{{16|32}} %{{.+}}, [[AND]]
  // CHECK: ret i{{16|32}} [[RES]]
  return v >> c;
}
