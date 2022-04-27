// RUN: %clang_cc1 -fintel-compatibility -verify -triple x86_64-pc-linux-gnu -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

class A { public: ~A(); };

class B : private virtual :: A { public: ~B(); }; // expected-note {{declared private here}}

class C : public virtual :: B { public: ~C(); }; // expected-warning {{inherited virtual base class '::A' has private destructor}}

C::~C() {}
// CHECK: call void @_ZN1AD2Ev(%class.A* {{[^,]*}})
