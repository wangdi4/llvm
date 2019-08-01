// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -ast-dump -pedantic | FileCheck %s
// expected-no-diagnostics

void foo() {
  int i;
  int a[10], b[10];

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}UnrollAndJam Disable
  #pragma nounroll_and_jam
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}UnrollAndJam Enable
  #pragma unroll_and_jam
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}UnrollAndJamCount Numeric
  #pragma unroll_and_jam (4)
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
}
