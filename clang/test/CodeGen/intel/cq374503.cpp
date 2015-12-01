//RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

//CHECK: %{{.*}} = type { i32 }
//CHECK: %{{.*}} = type { [4 x i8] }

class C {
  int field;

public:
  void foo() {
    struct A {
      char d[sizeof(*this)];
    };
    A ff;
  }
};

void test() {
  C x;
  x.foo();
}
