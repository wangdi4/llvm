// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// RUN: %clang_cc1 -fhls -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s

void foo() {
  int i;
  int a[10], b[10];

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}Fusion Disable
  #pragma nofusion
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // expected-error@+2 {{expected a for, while, or do-while loop to follow '#pragma nofusion'}}
  #pragma nofusion
  i = 7;
  for (i = 0; i < 10; ++i) {
    a[i] = b[i] = 0;
  }
}
