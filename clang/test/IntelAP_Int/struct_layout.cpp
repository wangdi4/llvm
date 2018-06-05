// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-windows-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=CHECK,WIN
// RUN: %clang -cc1 -O3 -disable-llvm-passes -triple x86_64-linux-pc -fhls %s -emit-llvm -o - | FileCheck %s --check-prefixes=CHECK,LIN


typedef int int33_tt __attribute__((__ap_int(33)));
typedef int int65_tt __attribute__((__ap_int(65)));

// CHECK: %class.Base33 = type { i33 }
// WIN: %class.Base33Plus = type { i33, i8 }
// LIN: %class.Base33Plus = type <{ i33, i8, [7 x i8] }>
// CHECK: %class.Base65 = type { i65 }
// WIN: %class.Derived = type { %class.Base33, %class.Base33Plus, %class.Base65 }
// LIN: %class.Derived = type { %class.Base33, %class.Base33Plus.base, %class.Base65 }
// LIN: %class.Base33Plus.base = type <{ i33, i8 }>
class Base33 {
  int33_tt an_int;
public:
  Base33(int init) : an_int(init){}
};
class Base33Plus {
  int33_tt an_int;
  char c;
public:
  Base33Plus(int init) : an_int(init), c(0){}
};

class Base65 {
  int65_tt an_int;
public:
  Base65(int init) : an_int(init){}
};

class Derived : public Base33, Base33Plus, Base65{
public:
  Derived(int init) : Base33(init), Base33Plus(init), Base65(init) {}
};

void foo() {
  Base33 B33 = 1;
  Base33Plus BP33 = 2;
  Base65 BP65 = 3;
  Derived D = 9;
}
