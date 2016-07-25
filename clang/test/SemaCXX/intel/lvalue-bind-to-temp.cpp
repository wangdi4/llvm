// RUN: %clang_cc1 -std=c++11 -fintel-ms-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -std=c++11 -fsyntax-only -verify -DERROR %s

struct A {
  int a;
  A(int x = 0) : a(x) {}
};

struct B {
  int b;
  B(int x = 0) : b(x) {}
};

struct Derived : public A {
  float a;
  Derived() : A(0), a(0.0) {}
};

// Test return values.
namespace ReturnValues {
// Test references to non-record type.
int &f1(int x) {
  return x ? 1 : 2; // expected-error{{non-const lvalue reference}}
}
// Test reference to a record type.
A &f2(int x) {
  return x ? A(1) : A(2);
#if ERROR
  // expected-error@-2{{non-const lvalue reference}}
#else
  // expected-warning@-4{{non-const lvalue reference}}
  // expected-warning@-5{{returning reference to local}}
#endif
}
// Test binding to the result of 'new' statement.
int *&f3(int x) {
  return new int[10];
#if ERROR
  // expected-error@-2{{non-const lvalue reference}}
#else
  // expected-warning@-4{{non-const lvalue reference}}
  // expected-warning@-5{{returning reference to local}}
#endif
}
// Test binding to unrelated object.
A &f4() {
  return B(1); // expected-error{{non-const lvalue reference}}
}
// Test binding to temporary of derived class.
A &f5() {
  return Derived();
#if ERROR
  // expected-error@-2{{non-const lvalue reference}}
#else
  // expected-warning@-4{{non-const lvalue reference}}
  // expected-warning@-5{{returning reference to local}}
#endif
}
// Test volatile reference to record type.
const volatile A &f6() {
  return A();
#if ERROR
  // expected-error@-2{{volatile lvalue reference}}
#else
  // expected-warning@-4{{volatile lvalue reference}}
  // expected-warning@-5{{returning reference to local}}
#endif
}
// Test templates.
template<class T>
T &f7(int x) {
  return x ? T(1) : T(2); // expected-error{{non-const lvalue reference to type 'int'}}
#if ERROR
  // expected-error@-2{{non-const lvalue reference to type 'A'}}
#else
  // expected-warning@-4{{non-const lvalue reference to type 'A'}}
  // expected-warning@-5{{returning reference to local}}
#endif
}
// Test universal reference.
template <class T>
T &&f8(int x) {
  return x ? T(1) : T(2); // expected-warning2{{returning reference to local}}
}

void run() {
  auto &x1 = f7<int>(0);  // expected-note{{in instantiation of}}
  auto &x2 = f7<A>(1);    // expected-note{{in instantiation of}}
  auto &&x3 = f8<int>(2); // expected-note{{in instantiation of}}
  auto &&x4 = f8<A>(3);   // expected-note{{in instantiation of}}
}

} // namespace ReturnValues

namespace Parameters {

void f1(int &x) {}                  // expected-note{{not viable}}
template<class T> void f5(T &&x) {}
#if ERROR
void f2(A &x) {}                    // expected-note3{{not viable}}
void f3(const volatile A &x) {}     // expected-note{{not viable}}
template<class T> void f4(T &x) {}  // expected-note2{{not viable}}
#else
void f2(A &x) {}                    // expected-note{{not viable}}
void f3(const volatile A &x) {}
template<class T> void f4(T &x) {}  // expected-note{{not viable}}
#endif

void run() {
  int x;
  f1(x ? 1 : 2);  // expected-error{{no matching function}}
  f2(B(0));       // expected-error{{no matching function}}
  f4(x ? 1 : 2);  // expected-error{{no matching function}}
  f5(x ? 1 : 2);
  f5(A(0));
#if ERROR
  f2(A(0));       // expected-error{{no matching function}}
  f2(Derived());  // expected-error{{no matching function}}
  f3(A(0));       // expected-error{{no matching function}}
  f4(A(0));       // expected-error{{no matching function}}
#else
  f2(A(0));       // expected-warning{{non-const lvalue reference}}
  f2(Derived());  // expected-warning{{non-const lvalue reference}}
  f3(A(0));       // expected-warning{{volatile lvalue reference}}
  f4(A(0));       // expected-warning{{non-const lvalue reference}}
#endif
}

} // namespace Parameters

namespace Overloading {
// Test that non-permissive candidate is better for overloading resolution.
void f1(int &x) {}
void f1(const int &x) {}
void f1(int &&x) {}

void f2(A &x) {}        // Viable, permissive.
void f2(const A &x) {}  // Viable, standard.
void f2(A &&x) {}

class X1 {
  X1(const X1 &other) = delete; // expected-note2{{explicitly marked deleted here}}
public:
  X1() {}
  X1(X1 &other);
};

X1 getX1() { return X1(); } // expected-error{{call to deleted constructor}}

void run() {
  int x;
  f1(x ? 1 : 2);
  f2(A(0));

  X1 x1(getX1()); // expected-error{{call to deleted constructor}}
}

} // namespace Overloading

namespace Other {

struct C {
  A a1;
  A a2;
};

void run() {
  int x;
  int &x1 = x ? 1 : 2;    // expected-error{{non-const lvalue reference}}
  int &x2{ 1 };           // expected-error{{non-const lvalue reference}}
  C &c1{ A(0), A(1) };    // expected-error{{non-const lvalue reference}}
#if ERROR
  A &a1 = A(0);           // expected-error{{non-const lvalue reference}}
  A &a2{ A(0) };          // expected-error{{non-const lvalue reference}}
  int *&p1 = new int[10]; // expected-error{{non-const lvalue reference}}
#else
  A &a1 = A(0);           // expected-warning{{non-const lvalue reference}}
  A &a2{ A(0) };          // expected-warning{{non-const lvalue reference}}
  int *&p1 = new int[10]; // expected-warning{{non-const lvalue reference}}
#endif
}

} // namespace Other

