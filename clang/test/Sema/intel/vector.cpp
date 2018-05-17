// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fintel-compatibility-enable=UnrollExtensions -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// expected-no-diagnostics

void foo(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}Vectorize Disable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma novector
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
  #pragma novector
  i = 7;
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}Vectorize Disable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
}
