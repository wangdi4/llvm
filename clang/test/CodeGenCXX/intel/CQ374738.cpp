// RUN: %clang_cc1 %s -std=c++11 -O0 -fintel-compatibility -verify -triple=i686-linux-gnu -emit-llvm -o - | FileCheck %s

int foo() {
  if (char *sss = static_cast<char *>("zzz")) { // expected-warning {{in Intel/GCC compatibility mode only}}
    if (*sss == 'z')
      return 0;
  }
  return 1;
}

// CHECK-LABEL: foo
// CHECK: store i8* {{.+}} [4 x i8]*

struct Base {
  int x;
};

template <typename T>
struct A {
  static const int N = sizeof(static_cast<Base *>(T())); // expected-warning {{in Intel/GCC compatibility mode only}}
};

struct Derived : Base {
  A<const Derived *> a; // expected-note {{requested here}}
};

int bar() {
  Derived d;
  return d.a.N;
}
