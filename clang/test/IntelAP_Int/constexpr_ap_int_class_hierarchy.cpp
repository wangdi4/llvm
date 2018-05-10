// Ensure we can declare a class hierarch with constexpr constructors
// where the base class has an ap_int member.

// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=WIN,CHECK
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=WIN,CHECK
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=LIN,CHECK
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls -std=c++17 %s -emit-llvm -o - | FileCheck %s --check-prefixes=LIN,CHECK


typedef int int65_tt __attribute__((__ap_int(65)));

class Base {
  int65_tt an_int;
public:
  constexpr Base(int init) : an_int(init) {}
};

class Derived : public Base {
public:
  constexpr Derived(int init) : Base(init) {}
};

//WIN: %class.Derived = type { %class.Base }
//WIN: %class.Base = type { i65 }
//LIN: %class.Derived = type { %class.Base.base, [7 x i8] }
//LIN: %class.Base.base = type { [9 x i8] }

constexpr Derived a_derived = 17;
//CHECK: @[[VAR:[a-zA-Z0-9_]+]] = internal constant { i65 } { i65 17 }, align 8

Derived foo() {
  return a_derived;
}
