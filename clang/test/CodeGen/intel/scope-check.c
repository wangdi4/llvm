// RUN: %clang_cc1 -fsyntax-only -verify -fblocks -fintel-compatibility -std=gnu99 -Wno-unreachable-code -S -emit-llvm -o - %s | FileCheck %s

int test3a() {
  goto L; // no diagnostic
int a;    // no diagnostic
L:
  return a;
}

// CHECK-LABEL: test3a
// CHECK: %[[NAME:.+]] = alloca
// CHECK-NEXT: br label %[[LABEL:.+]]
// CHECK: [[LABEL]]
// CHECK: load {{.+}} %[[NAME]]


void test3clean(int*);

int test3() {
  goto L; // expected-warning {{jump from this goto statement to its label}}
int a __attribute((cleanup(test3clean))); // expected-note {{jump bypasses initialization of variable with __attribute__((cleanup))}}
L:
  return a;
}

