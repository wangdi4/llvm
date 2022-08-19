// RUN: %clang_cc1 -fintel-compatibility -std=c++11 -ast-print \
// RUN:  %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility -std=c++11  -emit-pch -o %t %s

// RUN: %clang_cc1 -fintel-compatibility -std=c++11 -include-pch %t -ast-print \
// RUN:  %s | FileCheck %s

// expected-no-diagnostics

#ifndef HEADER
#define HEADER
struct S {
  int a;
  S();
  ~S();
  template <int t>
  void apply() {
#pragma vector temporal
    for (int i = 0; i < 1024; i++) {
      a++;
    }
#pragma vector nontemporal
    for (int i = 0; i < 1024; i++) {
      a++;
    }
#pragma vector vectorlength(t, 2)
    for (int i = 0; i < 1024; i++) {
      a++;
    }
  }
// CHECK: template <int t> void apply()
// CHECK: #pragma vector temporal(enable)
// CHECK: #pragma vector nontemporal(enable)
// CHECK: #pragma vector vectorlength(t)
// CHECK: #pragma vector vectorlength(2)
// CHECK: template<> void apply<10>()
// CHECK: #pragma vector temporal(enable)
// CHECK: #pragma vector nontemporal(enable)
// CHECK: #pragma vector vectorlength(10)
// CHECK: #pragma vector vectorlength(2)

};

template<typename T>
void foo(T x) {

#pragma vector temporal
  for (int i = 0; i < 1024; i++)
    x++;
#pragma vector nontemporal
  for (int i = 0; i < 1024; i++)
    x++;
#pragma vector vectorlength(2,4)
  for (int i = 0; i < 1024; i++)
    x++;
}
// CHECK: template <typename T> void foo(T x)
// CHECK: #pragma vector temporal(enable)
// CHECK: #pragma vector nontemporal(enable)
// CHECK: #pragma vector vectorlength(2)
// CHECK: #pragma vector vectorlength(4)
// CHECK: template<> void foo<int>(int x)
// CHECK: #pragma vector temporal(enable)
// CHECK: #pragma vector nontemporal(enable)
// CHECK: #pragma vector vectorlength(2)
// CHECK: #pragma vector vectorlength(4)
void use_template() {
   S obj;
   obj.apply<10>();
   foo(10);
}
#endif
