// RUN: %clang_cc1 %s -fexceptions -O0 -fintel-compatibility -emit-llvm -o - | FileCheck %s

struct X {
  static void foo();
} x;
struct Y {
  static void bar();
  static void bar(int);
} y;
// This is (presumably) valid, because x.f does not refer to an overloaded
// function name.
void (*ptr1)() = &x.foo;
void (*ptr2)() = &y.bar;
void (*ptr3)() = y.bar;

struct X *xptr = &x;
void (*p1)() = &xptr->foo;

struct Y *yptr = &y;
void (*ptr4)() = &yptr->bar;
void (*ptr5)(int) = &yptr->bar;

bool foo() {
  return ptr4 == &Y::bar;
}

// CHECK: @{{.*}}ptr1{{.*}} = global {{.+}}@[[PTR1:.+]],
// CHECK:  store {{.+}}[[PTR1]],{{.*}}@{{.*}}p1
