// RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-apple-darwin -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma bss_seg test

// CHECK: target
void www();

// CHECK: @a = global i32 0, align 4
// CHECK-NEXT: @b = global i32 5, align 4
// CHECK-NEXT: @c = global i32 0, align 4
// CHECK-NEXT: @d = global i32 5, align 4
// CHECK-NEXT: @e = constant i32 0, section "#bss#__DATA, sect1~@~", align 4
// CHECK-NEXT: @f = global i32 5, align 4
// CHECK-NEXT: @g = global %struct.S zeroinitializer, align 4
// CHECK-NEXT: @h = global %class.C zeroinitializer, align 4
// CHECK-NEXT: @push1 = global i32 0, section "#bss#__DATA, sect4~@~", align 4
// CHECK-NEXT: @pop1 = constant i32 0, section "#bss#__DATA, sect43~@~www", align 4
// CHECK-NEXT: @pop2 = global i32 0, align 4
// CHECK-NEXT: @_ZZ4mainE6localS = internal global i32 0, align 4

#pragma bss_seg ; // expected-warning {{extra text after expected end of preprocessing directive}}
int a;
int b = 5;
#pragma bss_seg (
int c, d = 5;
#pragma bss_seg ("sect1" 
int e, f=5;
struct S {
#pragma bss_seg ("sect2", "www"
  int a;
} g;
#pragma bss_seg (push, "sect43", "www") dfewer // expected-warning {{extra text after expected end of preprocessing directive}}


#pragma bss_seg (push, id1, "sect2", "www")
#pragma bss_seg (push, id2, "sect3", "") // expected-warning {{invalid use of null string; pragma ignored}}
#pragma bss_seg (pop, "") // expected-warning {{invalid use of null string; pragma ignored}}
class C {
  int a;
  public:
  int b;
} h;

#pragma bss_seg (push, "sect4")
int push1;
#pragma bss_seg (pop, id1)
int pop1;
#pragma bss_seg (pop)
int pop2;
// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
  return (0);
}

