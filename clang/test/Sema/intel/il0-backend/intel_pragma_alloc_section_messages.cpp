// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma alloc_section test

void www();

#pragma alloc_section ; // expected-warning {{missing '(' after '#pragma alloc_section' - ignoring}}
struct S {
  #pragma alloc_section (a, recursive // expected-error {{use of undeclared identifier 'a'}} expected-error {{use of undeclared identifier 'recursive'}} expected-warning {{expected a string}}
  #pragma alloc_section (a, "long") dfgdfg // expected-error {{use of undeclared identifier 'a'}} expected-warning {{extra text after expected end of preprocessing directive}}
  int a;
  #pragma alloc_section (a, "short") dfgdfg // expected-warning {{name of variable is expected}} expected-warning {{extra text after expected end of preprocessing directive}}
} d;

#pragma alloc_section (e, www, www(), "short") // expected-error {{use of undeclared identifier 'e'}} expected-error {{expected identifier}}
class C {
  int a;
  public:
  int b;
} e;
#pragma alloc_section (e, "long") // expected-warning {{variable 'e' has no "C" linkage specification}}
#pragma alloc_section (e, "section") // expected-warning {{Unexpected section 'section'}}
#pragma alloc_section (ddddddd, "short") // expected-error {{use of undeclared identifier 'ddddddd'}}
extern "C" int ddddddd;
extern "C" int dd1;
#pragma alloc_section (ddddddd, d.a, "long") // expected-error {{expected identifier}}

int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
#pragma alloc_section (argc, i, "long") // expected-warning {{variable 'argc' has local storage}} expected-warning {{variable 'i' has local storage}}
#pragma alloc_section (ddddddd, dd1, localS, "short") // expected-warning {{variable 'localS' has no "C" linkage specification}} 
  
  return (i);
}

