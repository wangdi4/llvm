// CQ#369185
// RUN: %clang_cc1 -fsyntax-only -std=c++11 -fintel-compatibility -verify %s
// expected-no-diagnostics

// Simple typelist. Compile-time list of types.
template <typename... _Elements>
struct __reflection_typelist;

// Specialization for an empty typelist.
template <>
struct __reflection_typelist<> {
  typedef void empty;
};

// Partial specialization.
template <typename _First, typename... _Rest>
struct __reflection_typelist<_First, _Rest...> {
  typedef void empty;

  struct first {
    typedef _First type;
  };

  struct rest {
    typedef __reflection_typelist<_Rest...> type;
  };
};

template <typename _Tp>
struct bases {
  typedef __reflection_typelist<__bases(_Tp)...> type;
};

template <typename _Tp>
struct direct_bases {
  typedef __reflection_typelist<__direct_bases(_Tp)...> type;
};

struct A {};
struct B : virtual A {};
struct C : A {};
struct D : virtual A, B {};
struct E : C, virtual D {};
typedef C G;
typedef D I;
struct E2 : public G, private D {};
struct E3 : I, G {};
struct F {};

template<typename T, typename U>
struct is_same_type {
  static const bool value = false;
};
template <typename T>
struct is_same_type<T, T> {
  static const bool value = true;
};

void check_bases() {
  static_assert(is_same_type<bases<A>::type,
                __reflection_typelist<>>::value,
                "struct A has the wrong bases");

  static_assert(is_same_type<bases<B>::type,
                __reflection_typelist<struct A>>::value,
                "struct B has the wrong bases");

  static_assert(is_same_type<bases<C>::type,
                __reflection_typelist<struct A>>::value,
                "struct C has the wrong bases");

  static_assert(is_same_type<bases<D>::type,
                __reflection_typelist<struct A, struct B>>::value,
                "struct D has the wrong bases");

  static_assert(is_same_type<bases<E>::type,
                __reflection_typelist<struct A, struct C, struct B, struct D>>::value,
                "struct E has the wrong bases");

  static_assert(is_same_type<bases<E2>::type,
                __reflection_typelist<struct A, struct C, struct B,  struct D>>::value,
                "struct E2 has the wrong bases");

  static_assert(is_same_type<bases<E3>::type,
                __reflection_typelist<struct A, struct B, struct D, struct A, struct C>>::value,
                "struct E3 has the wrong bases");

  static_assert(is_same_type<bases<F>::type,
                __reflection_typelist<>>::value,
                "struct F has the wrong bases");
}

void check_direct_bases() {
  static_assert(is_same_type<direct_bases<A>::type,
                __reflection_typelist<>>::value,
                "struct A has the wrong direct bases");

  static_assert(is_same_type<direct_bases<B>::type,
                __reflection_typelist<struct A>>::value,
                "struct B has the wrong direct bases");

  static_assert(is_same_type<direct_bases<C>::type,
                __reflection_typelist<struct A>>::value,
                "struct C has the wrong direct bases");

  static_assert(is_same_type<direct_bases<D>::type,
                __reflection_typelist<struct A, struct B>>::value,
                "struct D has the wrong direct bases");

  static_assert(is_same_type<direct_bases<E>::type,
                __reflection_typelist<struct C, struct D>>::value,
                "struct E has the wrong direct bases");

  static_assert(is_same_type<direct_bases<E2>::type,
                __reflection_typelist<struct C, struct D>>::value,
                "struct E2 has the wrong direct bases");

  static_assert(is_same_type<direct_bases<E3>::type,
                __reflection_typelist<struct D, struct C>>::value,
                "struct E3 has the wrong direct bases");

  static_assert(is_same_type<direct_bases<F>::type,
                __reflection_typelist<>>::value,
                "struct F has the wrong direct bases");
}
