// RUN: %clang_cc1 -fintel-compatibility -std=c++11 -triple x86_64-unknown-linux-gnu -ast-dump -o - %s | FileCheck %s
template <typename... _Elements> struct typelist {
  void print(_Elements... elems);
};

template <typename _Tp> struct bases {
  typedef typelist<__bases(_Tp)...> type;
};

template <typename _Tp> struct direct_bases {
  typedef typelist<__direct_bases(_Tp)...> type;
};

template<typename T, typename U>
struct is_same_type {
  static const bool value = false;
};
template <typename T>
struct is_same_type<T, T> {
  static const bool value = true;
};

class Base1 {};
class Base2 {};
class Derived1 : virtual Base1, Base2 {};
class Derived2 : virtual Base1, Base2 {};
class Derived12 : Derived1, Derived2 {};

int main() {
  Base1 b1;
  Base2 b2;
  Derived1 d1;
  Derived2 d2;
  Derived12 d12;

  bases<Derived12>::type baselist;
  direct_bases<Derived12>::type direct_baselist;
  baselist.print(b1, b2, d1, b2, d2);
  direct_baselist.print(d1, d2);
}

// CHECK: struct typelist<class Base1, class Base2, class Derived1, class Base2, class Derived2>
// CHECK: struct typelist<class Derived1, class Derived2>
