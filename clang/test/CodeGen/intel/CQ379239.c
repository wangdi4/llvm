// RUN: %clang_cc1 -emit-llvm -o - -fintel-compatibility -fintel-ms-compatibility -triple=x86_64-pc-win32 %s -debug-info-kind=limited | FileCheck %s

// CHECK-LABEL: @main
int main() {
  static char char1;
// CHECK: store i8 97, i8* @main.char1, align 1, !dbg ![[DBG1:[0-9]+]]
  char1 = 'a';
// CHECK-NEXT:  ret i32 0, !dbg ![[DBG2:[0-9]+]]
}

// CHECK: ![[DBG1]] = !DILocation(line: 7,
// CHECK: ![[DBG2]] = !DILocation(line: 9,

