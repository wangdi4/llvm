// RUN: %clang_cc1 -fintel-compatibility-enable=AllowMissingTypename -emit-llvm -o - -std=c++11 -verify -o - %s | FileCheck %s

template <class T>
struct X
{
typedef T type1;
type1 foo2();
};

template<class T>
X<T>::type1 X<T>::foo2() // expected-warning {{missing 'typename'}}
{
   type1 t;
   return t;
}

template<typename T>
void bar(int) {
  typedef T::type1 value_type; // expected-warning {{missing 'typename'}}
  X<T>::type1 var; // expected-warning {{missing 'typename'}}
}

// CHECK-LABEL: @main
int main() {
  X<float> x;
  // CHECK: call {{.*}}void {{.*}}bar
  bar<X<int>>(2);
  // CHECK: call {{.*}}float {{.*}}foo2
  return x.foo2();
}
