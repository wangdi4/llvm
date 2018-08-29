// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -ast-dump -verify | FileCheck %s

void foo() {
  int i, a[20], b[20], arr[20];
  const int N = 2;

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}IVDep Enable
  #pragma ivdep
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}IVDepBack Enable
  #pragma ivdep back
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}IVDepLoop Enable
  #pragma ivdep loop
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  #pragma ivdep loop
  #pragma ivdep
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}IVDepLoop Enable
  // CHECK: LoopHintAttr{{.*}}IVDep Enable
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
  #pragma ivdep
  #pragma ivdep loop
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}IVDep Enable
  // CHECK: LoopHintAttr{{.*}}IVDepLoop Enable
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;

  #pragma ivdep loop loop // expected-warning {{'loop' cannot appear multiple times in '#pragma ivdep' - ignored}}
  for (i = 0; i < 10; ++i)
    a[i] = a[i + N] * 5;
  #pragma ivdep back back // expected-warning {{'back' cannot appear multiple times in '#pragma ivdep' - ignored}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
#pragma ivdep loop back // expected-warning {{'back' may not appear in '#pragma ivdep' here - ignored}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
#pragma ivdep back loop // expected-warning {{'loop' may not appear in '#pragma ivdep' here - ignored}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
#pragma ivdep safelen(3) // expected-warning {{expected loop or back with '#pragma ivdep' - ignored}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
#pragma ivdep loop blah // expected-warning {{expected loop or back with '#pragma ivdep' - ignored}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;
}
