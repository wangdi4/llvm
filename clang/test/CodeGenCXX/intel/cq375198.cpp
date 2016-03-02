// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
struct A {
  void operator+();
};

typedef void (A::*P)();
template <P> struct S {};
template <typename T> void g(S < &T::operator+>);

int main() {
  S < &A::operator+> s;
  g<A>(s);
  // CHECK: call void @_Z1gI1AEv1SIXadsrT_plEE()
  return 0;
}

