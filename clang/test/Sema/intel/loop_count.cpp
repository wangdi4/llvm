// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -ast-dump -verify | FileCheck %s
#include <stdint.h>

void foo() {
  int i, a[20], b[20], arr[20];
  const int N = 2;
  int s;

  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}LoopCount Numeric
  // CHECK-NEXT: IntegerLiteral{{.*}}1
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCount Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}2
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCountMin Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}4
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCountMax Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}5
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCountAvg Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}2
  #pragma loop_count(1,2) min=4 max(5) avg=2
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}LoopCount Numeric
  // CHECK-NEXT: IntegerLiteral{{.*}}1
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCount Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}4
  #pragma loop_count=1,4
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  // CHECK: AttributedStmt
  // CHECK-NEXT: LoopHintAttr{{.*}}LoopCountMin Numeric
  // CHECK-NEXT: IntegerLiteral{{.*}}4
  // CHECK-NEXT-NEXT: LoopHintAttr{{.*}}LoopCountMax Numeric
  // CHECK-NEXT-NEXT: IntegerLiteral{{.*}}5
  #pragma loop_count min=4 max(5)
  for (i = 0; i < 10; ++i) {  // this is OK
    a[i] = b[i] = 0;
  }
  #pragma loop_count=1,2 min=4 max(5)
  #pragma loop_count(3)  // expected-error {{duplicate directives}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;

  #pragma loop_count min=4 max(5)
  #pragma loop_count min=6  // expected-error {{duplicate directives}}
  for (i = 0; i < 10; ++i)
    arr[i] = arr[i + N] * 5;

   // expected-error@+1 {{invalid value '0'; must be positive}}
   #pragma loop_count(0)
   for (int i = 0; i < 10; ++i){s = s + i;}

   // expected-error@+1 {{invalid value '-3'; must be positive}}
   #pragma loop_count(-3)
   for (int i = 0; i < 10; ++i){s = s + i;}

   // expected-error@+1 {{value '1844674407370955148' is too large}}
   #pragma loop_count(1844674407370955148)
   for (int i = 0; i < 10; ++i){s = s + i;}

   // expected-error@+1 {{duplicate directives 'loop_count min(1)' and 'loop_count min(1)'}}
   #pragma loop_count(1,2) min=1 max=10 avg=5 min = 1
   for (int i = 0; i < 10; ++i){s = s + i;}

   // expected-error@+1 {{duplicate directives 'loop_count avg(5)' and 'loop_count avg(1)'}}
   #pragma loop_count(1,2); min=1; max=10; avg=5; avg = 1
   for (int i = 0; i < 10; ++i){s = s + i;}
}

template<int64_t L>
void size_test() {
  int s = 0;
   // expected-error@+3 {{invalid value '0'; must be positive}}
   // expected-error@+2 {{invalid value '-2'; must be positive}}
   // expected-error@+1 {{value '1844674407370955161' is too large}}
   #pragma loop_count(L)
   for (int i = 0; i < 10; ++i){s = s + i;}
}

template <typename T> // expected-note {{declared here}}
void bar() {
  int s = 0;
  #pragma loop_count(T) // expected-error {{'T' does not refer to a value}}
  for (int i = 0; i < 10; ++i){s = s + i;}
}

int main() {
  bar<int>();

  // expected-note@+1 {{in instantiation of function template specialization 'size_test<0>' requested here}}
  size_test<0>();
  // expected-note@+1 {{in instantiation of function template specialization 'size_test<-2>' requested here}}
  size_test<-2>();
  // expected-note@+1 {{in instantiation of function template specialization 'size_test<1844674407370955161>' requested here}}
  size_test<1844674407370955161>();
}
