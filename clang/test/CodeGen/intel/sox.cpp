// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -sox="command-line recorded here by driver" -emit-llvm %s -o - | FileCheck %s

int main() {
  return 0;
}
  // CHECK: !llvm.ident = !{!1, !2}
  // CHECK: !1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler
  // CHECK-SAME: "}
  // CHECK: !2 = !{!"command-line recorded here by driver"}
