// RUN: %clang_cc1 -triple x86_64-unknown-unknown %s -emit-llvm -o - | FileCheck %s

#define small __attribute__((mode(QI))) int

int foo() {
  int x, y = 0x400;
  x = (small)y;
  if (sizeof(small) != sizeof(char))
    return 1;
  if (sizeof(x) != sizeof(char) && x == y)
    return 1;
  return 0;
}

// CHECK-LABEL: foo
// CHECK: alloca i32
// CHECK: [[XNAME:%.+]] = alloca i32
// CHECK: [[YNAME:%.+]] = alloca i32
// CHECK: [[TMP:%.+]] = load i32{{.+}}[[YNAME]]
// CHECK: trunc i32 [[TMP]] to i8
// CHECK: sext i8 {{.+}} to i32
// CHECK: store i32 {{.+}}[[XNAME]]
