// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0  -verify -fopenmp -ferror-limit 100  -o - %s

// RUN: %clang_cc1 -triple x86_64-apple-macos10.7.0 -fopenmp  -verify -fopenmp-simd  -ferror-limit 100  -o - %s

#pragma omp groupprivate // expected-error {{expected '(' after 'groupprivate'}}
#pragma omp groupprivate( // expected-error {{expected identifier}} expected-error {{expected ')'}} expected-note {{to match this '('}}
#pragma omp groupprivate() // expected-error {{expected identifier}}
#pragma omp groupprivate(1) // expected-error {{expected unqualified-id}}
struct CompleteSt{
 int a;
};

struct CompleteSt1{
#pragma omp groupprivate(1) // expected-error {{expected unqualified-id}}
 int a;
// expected-error@+1 {{initializer for groupprivate variable 'd' is not allowed}}
} d; // expected-note {{'d' defined here}}

int a; // expected-note {{'a' defined here}}
//expected-warning@+1 {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate(a) allocate(a) // expected-error {{unexpected device_type clause}}
#pragma omp groupprivate(u) // expected-error {{use of undeclared identifier 'u'}}
// expected-note@+1 8 {{'#pragma omp groupprivate' is specified here}}
#pragma omp groupprivate(d, a)
int foo() { // expected-note {{declared here}}
  static int l;
#pragma omp groupprivate(l)) // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp target
  a++; //  expected-warning 3 {{groupprivate directive for variable 'a' is ignored for x86-64/host compilation}}
  return 1;
}

#pragma omp groupprivate (a) (
// expected-warning@-1 {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate (a) [ // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate (a) { // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate (a) ) // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate (a) ] // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate (a) } // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
// expected-warning@+1 {{extra tokens at the end of '#pragma omp groupprivate' are ignore}}
#pragma omp groupprivate a // expected-error {{expected '(' after 'groupprivate'}}
#pragma omp groupprivate(a // expected-error {{expected ')'}} expected-note {{to match this '('}}
#pragma omp groupprivate(a)) // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
int x, y;
#pragma omp groupprivate(x)) // expected-warning {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate(y)),
// expected-warning@-1 {{extra tokens at the end of '#pragma omp groupprivate' are ignored}}
#pragma omp groupprivate(a)
#pragma omp groupprivate(d.a) // expected-error {{expected identifier}}
#pragma omp groupprivate((float)a) // expected-error {{expected unqualified-id}}
int foa; // expected-note {{'foa' declared here}}
#pragma omp groupprivate(faa) // expected-error {{use of undeclared identifier 'faa'; did you mean 'foa'?}}
#pragma omp groupprivate(foo) // expected-error {{'foo' is not a global variable, static local variable or static data member}}
#pragma omp groupprivate (int a=2) // expected-error {{expected unqualified-id}}

struct IncompleteSt; // expected-note {{forward declaration of 'IncompleteSt'}}

extern IncompleteSt e;
#pragma omp groupprivate (e) // expected-error {{groupprivate variable with incomplete type 'IncompleteSt'}}
// expected-warning@+1 {{groupprivate directive for variable 'a' is ignored for x86-64/host compilation}}
int &f = a; // expected-error {{initializer for groupprivate variable 'f' is not allowed}}
#pragma omp groupprivate (f) // expected-note {{'#pragma omp groupprivate' is specified here}}

class TestClass {
  private:
    int a; // expected-note {{declared here}}
    static int b; // expected-note {{'b' declared here}}
    TestClass() : a(0){}
  public:
    TestClass (int aaa) : a(aaa) {}
#pragma omp groupprivate (b, a) // expected-error {{'a' is not a global variable, static local variable or static data member}}
} g(10); // expected-error {{initializer for groupprivate variable 'g' is not allowed}}
#pragma omp groupprivate (b) // expected-error {{use of undeclared identifier 'b'}}
#pragma omp groupprivate (TestClass::b) // expected-error {{'#pragma omp groupprivate' must appear in the scope of the 'TestClass::b' variable declaration}}
#pragma omp groupprivate (g) // expected-note {{'#pragma omp groupprivate' is specified here}}

namespace ns {
  int m;
#pragma omp groupprivate (m, m)
}
#pragma omp groupprivate (m) // expected-error {{use of undeclared identifier 'm'}}
#pragma omp groupprivate (ns::m)
#pragma omp groupprivate (ns:m) // expected-error {{unexpected ':' in nested name specifier; did you mean '::'?}}

const int h = 12; // expected-error  {{initializer for groupprivate variable 'h' is not allowed}}
const volatile int i = 10; // expected-error  {{initializer for groupprivate variable 'i' is not allowed}}
#pragma omp groupprivate (h, i) // expected-note 2 {{'#pragma omp groupprivate' is specified here}}


template <class T>
class TempClass {
  private:
    T a;
    TempClass() : a(){}
  public:
    TempClass (T aaa) : a(aaa) {}
    static T s;
#pragma omp groupprivate (s)
};
#pragma omp groupprivate (s) // expected-error {{use of undeclared identifier 's'}}

int o; // expected-note {{candidate found by name lookup is 'o'}}
#pragma omp groupprivate (o)
namespace {
int o; // expected-note {{candidate found by name lookup is '(anonymous namespace)::o'}}
#pragma omp groupprivate (o)
#pragma omp groupprivate (o)
}
#pragma omp groupprivate (o) // expected-error {{reference to 'o' is ambiguous}}
#pragma omp groupprivate (::o)

int main(int argc, char **argv) { // expected-note {{'argc' defined here}}

  int x, y; // expected-note 1 {{'y' defined here}}
  static double d1;
  static double d2;
  static double d3; // expected-note {{'d3' defined here}}
  static double d4;
  static TestClass LocalClass(y); // expected-error {{initializer for groupprivate variable 'LocalClass' is not allowed}}
#pragma omp groupprivate(LocalClass) // expected-note {{'#pragma omp groupprivate' is specified here}}

// expected-warning@+2 {{groupprivate directive for variable 'd' is ignored for x86-64/host compilation}}
// expected-warning@+1 {{groupprivate directive for variable 'a' is ignored for x86-64/host compilation}}
  d.a = a;
  d2++;
  ;
#pragma omp groupprivate(argc+y) // expected-error {{expected identifier}}
#pragma omp groupprivate(argc,y) // expected-error 2 {{arguments of '#pragma omp groupprivate' must have static storage duration}}
#pragma omp groupprivate(d2) // expected-error {{'#pragma omp groupprivate' must precede all references to variable 'd2'}}
#pragma omp groupprivate(d1)
  {
// expected-warning@+1 {{groupprivate directive for variable 'a' is ignored for x86-64/host compilation}}
  ++a;d2=0;
#pragma omp groupprivate(d3) // expected-error {{'#pragma omp groupprivate' must appear in the scope of the 'd3' variable declaration}}
  }
#pragma omp groupprivate(d3)
label:
#pragma omp groupprivate(d4) // expected-error {{'#pragma omp groupprivate' cannot be an immediate substatement}}

#pragma omp groupprivate(a) // expected-error {{'#pragma omp groupprivate' must appear in the scope of the 'a' variable declaration}}
  return (y);
#pragma omp groupprivate(d) // expected-error {{'#pragma omp groupprivate' must appear in the scope of the 'd' variable declaration}}
}

namespace {
  int ooo;
  // expected-note@+2 {{'#pragma omp groupprivate' is specified here}}
  // expected-note@+1 {{'#pragma omp groupprivate' is specified here}}
  #pragma omp groupprivate (ooo) device_type(nohost)
};

int xoo() {
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  ::ooo++;
// expected-warning@+1 {{groupprivate directive for variable 'ooo' is ignored for x86-64/host compilation}}
  return ::ooo;
}

// end INTEL_COLLAB
