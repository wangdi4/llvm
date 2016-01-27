// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: internal global i8* blockaddress(@foo, %[[LABEL:.+]]),

// CHECK_LABEL: @foo
void foo() {
  static void *v = &&r;
  if (v != &&r) {
  };
// CHECK: [[LABEL]]
r:;
}
