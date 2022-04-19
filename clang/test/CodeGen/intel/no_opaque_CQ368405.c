// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility -fms-compatibility %s -emit-llvm -no-opaque-pointers -debug-info-kind=limited -o - | FileCheck %s

// CHECK-LINE: main
int main()
{
  // CHECK: store i32 0, i32* %{{.+}}, !dbg ![[PROLOGUE:[0-9]+]]
  return 0;
}

// CHECK: ![[PROLOGUE]] = !DILocation(line: [[@LINE-5]],
