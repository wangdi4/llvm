// RUN: %clang_cc1 -fintel-compatibility -triple=x86_64-apple-darwin -emit-llvm -verify -o - %s | FileCheck %s  
//***INTEL: pragma init_seg test

// CHECK: target
void www();

// CHECK-NOT: define private void @.DIRECTIVE.
#pragma init_seg ; // expected-warning {{'compiler', 'lib', 'user' or section name is expected}}
#pragma init_seg ( // expected-warning {{'compiler', 'lib', 'user' or section name is expected}}
#pragma init_seg "" // expected-warning {{invalid use of null string; pragma ignored}}

int a, wed;
// CHECK: define i32 
int www1() 
{
  return (0);
}
// CHECK: define private void @.DIRECTIVE.() #1 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"INIT_SEG", metadata !".CRT$XCC")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
#pragma init_seg (compiler
struct S {
// CHECK-NOT: define private void @.DIRECTIVE..1() #1 {
#pragma init_seg lib // expected-warning {{initialization segment already defined}}
  int a;
} d;
#pragma init_seg(user) // expected-warning {{initialization segment already defined}}

#pragma init_seg "user") // expected-warning {{initialization segment already defined}}
class C {
  int a;
  public:
  int b;
} e;

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
  return (0);
}

