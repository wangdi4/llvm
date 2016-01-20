// RUN: %clang_cc1 -fintel-compatibility -verify -emit-llvm -o - %s | FileCheck %s

struct foo a[];   // expected-warning {{tentative array definition assumed to have one element}}
struct foo b[10]; // OK

struct foo {
  int x;
};

void bar() {
  struct foo s;
  a[0].x = 33;
  b[1].x = 77;
}

// CHECK @[[BNAME:.+]] = common global [10 x
// CHECK @[[ANAME:.+]] = common global [1 x
// CHECK-LABEL: bar
// CHECK: store {{.+}} @[[ANAME:.+]]
// CHECK: store {{.+}} @[[BNAME:.+]]
