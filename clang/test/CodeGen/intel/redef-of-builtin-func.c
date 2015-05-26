// CQ#368318
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

char index = 1;
int malloc = 2;

int check() {
  index = 42;
  return malloc;
}

// CHECK: @index = global i8 1, align 1
// CHECK: @malloc = global i32 2, align 4

// CHECK: store i8 42, i8* @index, align 1
// CHECK: %{{.+}} = load i32, i32* @malloc, align 4
