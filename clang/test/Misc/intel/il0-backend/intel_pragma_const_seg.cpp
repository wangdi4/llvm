// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple=x86_64-apple-darwin -verify -o - %s | FileCheck %s
//***INTEL: pragma const_seg test

// CHECK: target
void www();

// CHECK: @a = global i32 0, align 4
// CHECK-NEXT: @b = constant i32 5, align 4
// CHECK-NEXT: @c = constant i32 0, align 4
// CHECK-NEXT: @d = constant i32 5, align 4
// CHECK-NEXT: @e = constant i32 0, section "#const#__DATA, sect1~@~", align 4
// CHECK-NEXT: @f = constant i32 5, section "#const#__DATA, sect1~@~", align 4
// CHECK-NEXT: @g = constant %struct.S zeroinitializer, section "#const#__DATA, sect2~@~www", align 4
// CHECK-NEXT: @h = constant %class.C zeroinitializer, section "#const#__DATA, sect2~@~www", align 4
// CHECK-NEXT: @push1 = constant i32 1, section "#const#__DATA, sect4~@~", align 4
// CHECK-NEXT: @pop1 = constant i32 0, section "#const#__DATA, sect43~@~www", align 4
// CHECK-NEXT: @pop2 = constant i32 3, align 4
// CHECK-NEXT: @_ZZ4mainE6localS = internal global i32 0, align 4


#pragma const_seg ; // expected-warning {{extra text after expected end of preprocessing directive}}
int a;
const volatile int b = 5;
#pragma const_seg (
const volatile int c = 0, d = 5;
#pragma const_seg ("sect1" 
const volatile int e = 0, f=5;
struct S {
#pragma const_seg ("sect2", "www"
  int a;
  S() {}
};
const volatile S g;
#pragma const_seg (push, "sect43", "www") dfewer // expected-warning {{extra text after expected end of preprocessing directive}}


#pragma const_seg (push, id1, "sect2", "www")
#pragma const_seg (push, id2, "sect3", "") // expected-warning {{invalid use of null string; pragma ignored}}
#pragma const_seg (pop, "") // expected-warning {{invalid use of null string; pragma ignored}}
class C {
  int a;
  public:
  int b;
  C() {}
};
const volatile C h;

#pragma const_seg (push, "sect4")
const volatile int push1 = 1;
#pragma const_seg (pop, id1)
const volatile int pop1 = 0;
#pragma const_seg (pop)
const volatile int pop2 = 3;
// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
  i = b + c + d + e + f + g.a + h.b + push1 + pop1 + pop2;
  return (i);
}

