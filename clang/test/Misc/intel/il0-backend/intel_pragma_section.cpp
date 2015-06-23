// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma section test

void www();

#pragma section ; // expected-warning {{missing '(' after '#pragma section' - ignoring}}
#pragma section ( // expected-warning {{expected a string}}
// CHECK: define private void @.DIRECTIVE.() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SECTION", metadata !"sect1", metadata !"read", metadata !"write")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
#pragma section ("sect1" // expected-warning {{missing ')' after '#pragma section' - ignoring}}
struct S {
// CHECK: define private void @.DIRECTIVE..1() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SECTION", metadata !"sect1")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
  #pragma section ("sect1", a, // expected-warning {{identifier 'a' is undefined}} expected-warning {{missing ')' after '#pragma section' - ignoring}}
  int a;
// CHECK: define private void @.DIRECTIVE..2() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SECTION", metadata !"sect1", metadata !"read", metadata !"write", metadata !"execute", metadata !"shared", metadata !"nopage", metadata !"nocache", metadata !"discard", metadata !"remove")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
  #pragma section ("sect1", read, write, execute, shared, nopage, nocache, discard, remove, wed) // expected-warning {{identifier 'wed' is undefined}}
} d;

// CHECK: define private void @.DIRECTIVE..3() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SECTION", metadata !"section", metadata !"read", metadata !"short")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
#pragma section ("section", read, short, "section", write(), "section") wwewe // expected-warning {{missing ')' after '#pragma section' - ignoring}}
class C {
  int a;
  public:
  int b;
} e;
extern "C" int ddddddd;
extern "C" int dd1;

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SECTION", metadata !"sect1", metadata !"read", metadata !"write", metadata !"execute", metadata !"shared", metadata !"nopage", metadata !"nocache", metadata !"discard", metadata !"remove")
  #pragma section ("sect1", read, write, execute, shared, nopage, nocache, discard, remove, wed) // expected-warning {{identifier 'wed' is undefined}}
  
// CHECK: ret i32
  return (i);
}

