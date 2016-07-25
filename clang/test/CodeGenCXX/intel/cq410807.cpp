// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -std=c++11 -verify -fintel-compatibility -fintel-ms-compatibility -emit-llvm %s -o - | FileCheck %s

// A modified test from MS documentation (https://msdn.microsoft.com/en-us/library/dczztdfe.aspx)

namespace cq410807 {
class A {
public:
  void f(int) {}

  typedef void (A::*TAmtd)(int);

  struct B {
    TAmtd p;
  };

  void g() {
    B b = {f}; // expected-warning {{must explicitly qualify name of member function when taking its address}}
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.cq410807::A::B"* @_ZZN8cq4108071A1gEvE1b to i8*), i64 16, i32 8, i1 false)
    B b2 = {&A::f};
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.cq410807::A::B"* @_ZZN8cq4108071A1gEvE2b2 to i8*), i64 16, i32 8, i1 false)
    B b3 = {(TAmtd)f}; // expected-warning {{must explicitly qualify name of member function when taking its address}}
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.cq410807::A::B"* @_ZZN8cq4108071A1gEvE2b3 to i8*), i64 16, i32 8, i1 false)
  }
};

void foo() {
  A a;
  a.g();
}
}

// Test case from CQ#408058.
namespace cq408058 {
struct Base {};
struct Derived : Base {};

typedef void (Base::*PF)(void);

struct S { PF pfn; };

struct A {
  void f() {}
  static void g() {
    static const S s{ (PF)(void (Derived::*)(void))&f }; // expected-warning {{must explicitly qualify name of member function when taking its address}}
  }
};
}

int main()
{
  cq408058::A a;
}

