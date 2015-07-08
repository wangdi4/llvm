// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma alloc_text test

// CHECK: target
void www();

#pragma alloc_text ; // expected-warning {{missing '(' after '#pragma alloc_text' - ignoring}}
#pragma alloc_text ( // expected-warning {{expected a string}}
int a, wed;
#pragma alloc_text ("sect1" // expected-warning {{expected a function name}}
struct S {
  #pragma alloc_text ("sect1", a, // expected-warning {{missing ')' after '#pragma alloc_text' - ignoring}} // expected-warning {{expected a function name}}
  int a;
  #pragma alloc_text ("sect1", wed) // expected-warning {{expected a function name}}
} d;


extern "C" int correct_www_no_body();
#pragma alloc_text ("sect1", correct_www_no_body) 
#pragma alloc_text ("sect1", www) // expected-warning {{function 'www' has no "C" linkage specification}}
#pragma alloc_text ("sect1", www) // expected-warning {{function 'www' has no "C" linkage specification}}
class C {
  int a;
  public:
  int b;
} e;
extern "C" int ddddddd;
extern "C" int dd1;

// CHECK: define i32 @correct_www() {{.*}} {
extern "C" int correct_www() {
  return (correct_www_no_body());
}

  #pragma alloc_text ("sect1", correct_www) 
  #pragma alloc_text ("sect1", correct_www) // expected-warning {{text segment already specified}}
// CHECK: define i32 @correct_www_no_body() {{.*}} section "__TEXT, sect1" {
extern "C" int correct_www_no_body() {
  return 0;
}

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
  return (correct_www());
}

