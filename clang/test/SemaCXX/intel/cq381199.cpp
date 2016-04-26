// RUN: %clang_cc1 -fsyntax-only -std=c++11 -fintel-compatibility -verify %s
// expected-no-diagnostics

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

  static_assert(is_same_type<bases<Derived12>::type,
                typelist<struct Base1, struct Base2, struct Derived1,
                         struct Base2, class Derived2>>::value,
                "bases<Derived12>::type must be typelist<struct Base1, "
                "struct Base2, struct Derived1, struct Base2, "
                "class Derived2>>::type");
  static_assert(is_same_type<direct_bases<Derived12>::type,
                typelist<struct Derived1, class Derived2>>::value,
                "bases<Derived12>::type must be typelist"
                "<struct Derived1, struct Base2, class Derived2>>::type");
  bases<Derived12>::type baselist;
  direct_bases<Derived12>::type direct_baselist;
  baselist.print(b1, b2, d1, b2, d2);
  direct_baselist.print(d1, d2);
}
