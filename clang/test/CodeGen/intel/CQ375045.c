// REQUIRES: llvm-backend
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -triple i686-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  // CHECK: [[RHS:%.+]] = sext i8
  // CHECK: [[AND:%.+]] = and i{{16|32}} %conv, {{15|31}}
  // CHECK: [[RES:%.+]] = shl i{{16|32}} %{{.+}}, [[AND]]
  // CHECK: ret i{{16|32}} [[RES]]
  return argc << argv[0][0];
}

