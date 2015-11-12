//RUN: %clang_cc1 -fintel-compatibility -DTEMPLATE_WITH_INSTANTIATION -verify %s
//RUN: %clang_cc1 -fintel-compatibility -DTEMPLATE -verify %s
//RUN: %clang_cc1 -fintel-compatibility -DNON_TEMPLATE -verify %s

class Incomplete;
class Complete { int x; };
class Complete2 { double y; };

class Base {
public:
  int a;
  virtual const Incomplete *f1() = 0;
  virtual Complete *f2() = 0;
  virtual Complete *f3() = 0;
  virtual Complete f4() { return Complete(); }
#if TEMPLATE_WITH_INSTANTIATION || NON_TEMPLATE
// expected-note@-4{{is here}}
// expected-note@-4{{is here}}
// expected-note@-4{{is here}}
#else
// expected-no-diagnostics
#endif
};


#if TEMPLATE
// Template derived class, but without real instantiation. No diagnostics,
// because return types of overriding virtual functions are NOT checked.
template <class T>
class NoInstantiation : public Base {
#elif TEMPLATE_WITH_INSTANTIATION
// Template derived class with instantion, return types of overriding functions
// are checked.
template <class T>
class HasInstantiation : public Base {
#elif NON_TEMPLATE
// Non-template derived class, return types of overriding functions are checked.
class NonTemplate : public Base {
#endif
public:
  // OK, because return type of f1() is less qualified than type of Base::f1(),
  // unqualified return types match. Completeness of types is not checked here.
  virtual Incomplete *f1() {}
  // Error: overriding functions are more qualified than functions in Base.
  virtual Complete * const f2() {} // const pointer     vs  non-const pointer
  virtual const Complete *f3() {}  // pointer to const  vs  pointer to non-const
  // Error: different return types.
  virtual Complete2 f4() { return Complete2(); }
#if TEMPLATE_WITH_INSTANTIATION || NON_TEMPLATE
  // expected-error@-5{{return type of virtual function 'f2' is not covariant}}
  // expected-error@-5{{return type of virtual function 'f3' is not covariant}}
  // expected-error@-4{{virtual function 'f4' has a different return type}}
#endif
};

int main() {
#if TEMPLATE
  return 0;
#elif TEMPLATE_WITH_INSTANTIATION
  HasInstantiation<int> obj; // expected-note{{in instantiation of}}
  return obj.a;
#elif NON_TEMPLATE
  NonTemplate obj;
  return obj.a;
#else
#error Unknown test mode!
#endif
}
