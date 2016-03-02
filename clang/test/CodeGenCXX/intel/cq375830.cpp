// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fsyntax-only -fintel-compatibility -verify -emit-llvm %s -o - | FileCheck %s
//
// CQ#375830: xmain compiler should propogate '__restrict' attribute from
// the old declaration to the new. On windows there is a difference in
// name mangling.
//
// expected-no-diagnostics

class C {
  void foo();
  void bar() __restrict;
};

void C::foo() __restrict { ; }
// CHECK: define void @"\01?foo@C@@AEAAXXZ"(%class.C* %this)
void C::bar() { ; }
// CHECK: define void @"\01?bar@C@@AEIAAXXZ"(%class.C* %this)

