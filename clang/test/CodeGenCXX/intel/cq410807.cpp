// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -fintel-ms-compatibility -emit-llvm %s -o - | FileCheck %s

// A modified test from MS documentation (https://msdn.microsoft.com/en-us/library/dczztdfe.aspx)

class A {
public:
  void f(int) {}

  typedef void (A::*TAmtd)(int);

  struct B {
    TAmtd p;
  };

  void g() {
    B b = {f};
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.A::B"* @_ZZN1A1gEvE1b to i8*), i64 16, i32 8, i1 false)
    B b2 = {&A::f};
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.A::B"* @_ZZN1A1gEvE2b2 to i8*), i64 16, i32 8, i1 false)
    B b3 = {(TAmtd)f};
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* {{%.*}}, i8* bitcast (%"struct.A::B"* @_ZZN1A1gEvE2b3 to i8*), i64 16, i32 8, i1 false)
  }
};

void foo() {
  A a;
  a.g();
}

