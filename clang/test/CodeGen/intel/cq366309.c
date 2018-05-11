// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: @{{.+}} = common global [3 x %struct.A]*
struct A (*x)[3];

struct A {
  int a;
  int b;
};
