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
  // expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma vector'}}
  #pragma vector
  i = 7;
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
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeAlways Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector always
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeAlways Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeAlwaysAssert Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector always assert
  for (i = 0; i < 10; ++i) {  // this is OK
    x[i] = y[i];
  }
}

void goo(int i, int *x, int *y) {

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeAligned Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector aligned
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeDynamicAlign Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector dynamic_align
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeNoDynamicAlign Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector nodynamic_align
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeVecremainder Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector vecremainder
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeNoVecremainder Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector novecremainder
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeTemporal Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector temporal
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeNonTemporal Enable
  // CHECK-NEXT: NULL
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector nontemporal
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeLength Numeric
  // CHECK-NEXT: IntegerLiteral{{.*}} 2
  // CHECK-NEXT: NULL
  // CHECK-NEXT: LoopHintAttr{{.*}}VectorizeLength Numeric
  // CHECK-NEXT: IntegerLiteral{{.*}} 4
  // CHECK-NEXT: NULL
  // CHECK-NEXT: ForStmt
  #pragma vector vectorlength(2,4)
  for (i = 0; i < 10; ++i) {
    x[i] = y[i];
  }
}
