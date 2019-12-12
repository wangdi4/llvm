//RUN: %clang_cc1 %s -cl-std=clc++ -pedantic -ast-dump -verify | FileCheck %s

// INTEL_CUSTOMIZATION
//CHECK: CXXMethodDecl {{.*}} constexpr operator() 'int (int){{( __attribute__.*)?}} const __generic'
// end INTEL_CUSTOMIZATION
auto glambda = [](auto a) { return a; };

__kernel void test() {
  int i;
// INTEL_CUSTOMIZATION
//CHECK: CXXMethodDecl {{.*}} constexpr operator() 'void (){{( __attribute__.*)?}} const __generic'
// end INTEL_CUSTOMIZATION
  auto  llambda = [&]() {i++;};
  llambda();
  glambda(1);
  // Test lambda with default parameters
// INTEL_CUSTOMIZATION
//CHECK: CXXMethodDecl {{.*}} constexpr operator() 'void (){{( __attribute__.*)?}} const __generic'
// end INTEL_CUSTOMIZATION
  [&] {i++;} ();
  __constant auto err = [&]() {}; //expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('__constant (lambda at {{.*}})'), parameter type must be 'const __generic (lambda at {{.*}})'}}
  err();                          //expected-error-re{{no matching function for call to object of type '__constant (lambda at {{.*}})'}}
  // FIXME: There is very limited addr space functionality
  // we can test when taking lambda type from the object.
  // The limitation is due to addr spaces being added to all
  // objects in OpenCL. Once we add metaprogramming utility
  // for removing address spaces from a type we can enhance
  // testing here.
  (*(__constant decltype(llambda) *)nullptr)(); //expected-error{{multiple address spaces specified for type}}
  (*(decltype(llambda) *)nullptr)();
}

__kernel void test_qual() {
// INTEL_CUSTOMIZATION
//CHECK: |-CXXMethodDecl {{.*}} constexpr operator() 'void (){{( __attribute__.*)?}} const'
// end INTEL_CUSTOMIZATION
  auto priv1 = []() __private {};
  priv1();
// INTEL_CUSTOMIZATION
//CHECK: |-CXXMethodDecl {{.*}} constexpr operator() 'void (){{( __attribute__.*)?}} const __generic'
// end INTEL_CUSTOMIZATION
  auto priv2 = []() __generic {};
  priv2();
  auto priv3 = []() __global {}; //expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('(lambda at {{.*}})'), parameter type must be 'const __global (lambda at {{.*}})'}} //expected-note{{conversion candidate of type 'void (*)()'}}
  priv3(); //expected-error{{no matching function for call to object of type}}

  __constant auto const1 = []() __private{}; //expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('__constant (lambda at {{.*}})'), parameter type must be 'const (lambda at {{.*}}'}} //expected-note{{conversion candidate of type 'void (*)()'}}
  const1(); //expected-error{{no matching function for call to object of type '__constant (lambda at}}
  __constant auto const2 = []() __generic{}; //expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('__constant (lambda at {{.*}})'), parameter type must be 'const __generic (lambda at {{.*}}'}} //expected-note{{conversion candidate of type 'void (*)()'}}
  const2(); //expected-error{{no matching function for call to object of type '__constant (lambda at}}
// INTEL_CUSTOMIZATION
//CHECK: |-CXXMethodDecl {{.*}} constexpr operator() 'void (){{( __attribute__.*)?}} const __constant'
// end INTEL_CUSTOMIZATION
  __constant auto const3 = []() __constant{};
  const3();

  [&] () __global {} (); //expected-error{{no matching function for call to object of type '(lambda at}} expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('(lambda at {{.*}})'), parameter type must be 'const __global (lambda at {{.*}})'}}
  [&] () __private {} (); //expected-error{{no matching function for call to object of type '(lambda at}} expected-note-re{{candidate function not viable: address space mismatch in 'this' argument ('(lambda at {{.*}})'), parameter type must be 'const (lambda at {{.*}})'}}

  [&] __private {} (); //expected-error{{lambda requires '()' before attribute specifier}} expected-error{{expected body of lambda expression}}

  [&] () mutable __private {} ();
  [&] () __private mutable {} (); //expected-error{{expected body of lambda expression}}
}

