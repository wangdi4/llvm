// CQ#372058
// RUN: %clang-cc1 -fcxx-exceptions -fexceptions -O0 -g -emit-llvm %s -o - | FileCheck %s

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
