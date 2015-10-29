// RUN: %clang_cc1 -triple x86_64-windows -fsyntax-only -verify -fblocks -fcxx-exceptions -fms-extensions -fintel-compatibility -std=gnu++11 -Wno-unreachable-code -S -emit-llvm -o - %s | FileCheck %s

namespace test8 {
  void test1(int c) {
    switch (c) {
    case 0:
      int x = 56; // expected-note {{jump bypasses variable initialization}}
    case 1:       // expected-warning {{jump from switch statement to this case label bypasses variable initialization}}
      x = 10;
    }
  }

// CHECK-LABEL: define {{.+}}test1{{.+}}test8
// CHECK: switch {{.+}}, label %[[EPILOG:.+]] [
// CHECK: [[EPILOG]]
  void test2() {
    goto l2;     // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
  l1: int x = 5; // expected-note {{jump bypasses variable initialization}}
  l2: x++;
  }
// CHECK-LABEL: define {{.+}}test2{{.+}}test8
// CHECK: %[[VAR:.+]] = alloca
// CHECK: br label %[[LABEL:.+]]
// CHECK: store {{.+}}[[VAR]]
// CHECK: [[LABEL]]
// CHECK: load {{.+}}%[[VAR]]
}

namespace test10 {
  int test() {
    static void *ps[] = { &&a0 };
    goto *&&a0; // expected-warning {{jump from this goto statement to its label is a Microsoft extension}}
    int a = 3; // expected-note {{jump bypasses variable initialization}}
  a0:
    return 0;
  }
}
// CHECK-LABEL: define {{.+}}test{{.+}}test10
// CHECK: %[[VAR:.+]] = alloca
// CHECK-NEXT: br label %[[LABEL:.*]]
// CHECK-NOT: store {{.+}}[[VAR]]
// CHECK: [[LABEL]]

