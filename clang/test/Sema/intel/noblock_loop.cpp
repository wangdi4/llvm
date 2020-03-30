// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify \
// RUN: -ast-dump -pedantic | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaBlockLoop \
// RUN: -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s

void foo(int i, int *x, int *y) {
  // CHECK: AttributedStmt
  // CHECK-NEXT: IntelBlockLoopAttr{{.*}} noblock_loop -1
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma noblock_loop
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
  // expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma noblock_loop'}}
  #pragma noblock_loop
  i = 7;
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void bar(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: IntelBlockLoopAttr{{.*}} noblock_loop -1
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma noblock_loop
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}
