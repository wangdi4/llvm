// RUN: %clang_cc1 -O2 -fintel-compatibility -triple x86_64-unknown-linux -emit-llvm %s -o - | FileCheck %s

int foo(int k) {
  __assume(k == 3);
  if (k > 2)
    return k + 1;
  else
    return k - 1;
}

// CHECK: ret i32 4

