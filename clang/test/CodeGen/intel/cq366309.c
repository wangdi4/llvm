// RUN: %clang_cc1 -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: @{{.+}} = common global [3 x %struct.A]*
struct A (*x)[3];

struct A {
  int a;
  int b;
};
