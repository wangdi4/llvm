// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility-enable=PragmaNoVector -fintel-compatibility-enable=PragmaVector -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s

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
  // expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma novector'}}
  #pragma novector
  i = 7;
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void bar(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}Vectorize Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void zoo(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}Vectorize Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeAlways Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector always
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}
