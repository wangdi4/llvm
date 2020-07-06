// CQ#372058
// RUN: %clang_cc1 -fcxx-exceptions -fexceptions -O0 -gno-column-info -debug-info-kind=limited -emit-llvm %s -o - | FileCheck %s

int main() {
  int i;
  try {
    i = 1;
    throw i;
  }
  catch (int k) {
    i = 2;
  }
  // CHECK: !DILocation(line: 9, scope: !{{.+}})
  // CHECK-NOT: !DILocation(line: 16, scope: !{{.+}})
  return i;
}
