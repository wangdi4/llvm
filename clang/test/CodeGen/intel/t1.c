// CQ#368123

// RUN: %clang_cc1 -fintel-compatibility -debug-info-kind=limited -emit-llvm %s -o - | FileCheck -check-prefix CHECK4 %s
// CHECK4: !llvm.dbg.cu = !{!0}


int main() {
  return 0;
}
