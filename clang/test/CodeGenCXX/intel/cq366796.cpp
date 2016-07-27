// RUN: %clang_cc1 -O0 -debug-info-kind=limited %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -O0 --no_expr_source_pos -debug-info-kind=limited %s -emit-llvm -o - | FileCheck %s -check-prefix=CHECK_NO_DBG

int f() { return 1; }
int g() { return 2; }
int foo(int a, int b) { return a + b; }

int TestAssignmentExpr() {
  // CHECK-DAG: !DILocation(line: [[@LINE+8]]
  // CHECK-DAG: !DILocation(line: [[@LINE+8]]
  // CHECK-DAG: !DILocation(line: [[@LINE+8]]

  // CHECK_NO_DBG: !DILocation(line: [[@LINE+4]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+4]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+4]]

  int a =     // debug loc always
    f() +     // NO debug loc with '--no_expr_source_pos'
    g()       // NO debug loc with '--no_expr_source_pos'
  ;
  return a;
}

int TestFunctionArgs() {
  // CHECK-DAG: !DILocation(line: [[@LINE+12]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]
  // CHECK-DAG: !DILocation(line: [[@LINE+10]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]

  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+6]]
  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+10]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+5]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+5]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+5]]

  int a =     // debug loc always
    foo(      // NO debug loc with '--no_expr_source_pos'
      f(),    // NO debug loc with '--no_expr_source_pos'
      g()     // NO debug loc with '--no_expr_source_pos'
    );
  return a;   // debug loc always
}

int TestWhileExpr() {
  // CHECK-DAG: !DILocation(line: [[@LINE+14]]
  // CHECK-DAG: !DILocation(line: [[@LINE+14]]
  // CHECK-DAG: !DILocation(line: [[@LINE+14]]
  // CHECK-DAG: !DILocation(line: [[@LINE+14]]
  // CHECK-DAG: !DILocation(line: [[@LINE+16]]
  // CHECK-DAG: !DILocation(line: [[@LINE+17]]

  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+7]]
  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+7]]
  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+11]]
  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+12]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+5]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+5]]

  int a = 0;    // debug loc always
  while (       // debug loc always
    f() > 2 ||  // NO debug loc with '--no_expr_source_pos'
    g() < 3     // NO debug loc with '--no_expr_source_pos'
  )
  {
    a = 1;      // debug loc always
  }
  return a;     // debug loc always
}

int TestReturnExpr() {
  // CHECK-DAG: !DILocation(line: [[@LINE+12]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]
  // CHECK-DAG: !DILocation(line: [[@LINE+13]]
  // CHECK-DAG: !DILocation(line: [[@LINE+9]]

  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+6]]
  // CHECK_NO_DBG-DAG: !DILocation(line: [[@LINE+6]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+6]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+6]]
  // CHECK_NO_DBG-NOT: !DILocation(line: [[@LINE+6]]

  int a = 0;  // debug loc always
  return      // debug loc always
    f() +     // NO debug loc with '--no_expr_source_pos'
    g() +     // NO debug loc with '--no_expr_source_pos'
    a;        // NO debug loc with '--no_expr_source_pos'
}
