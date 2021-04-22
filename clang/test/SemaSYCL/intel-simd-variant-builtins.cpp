// RUN: %clang_cc1 -verify -fsyntax-only -std=c++17 -Wno-c++20-extensions \
// RUN:  -fsycl-is-device -triple spir64-unknown-unknown-sycldevice \
// RUN:  %s

namespace std {

  template<typename _Tp> struct remove_reference { typedef _Tp type; };

  template<typename _Tp>
  struct add_pointer {
    typedef typename remove_reference<_Tp>::type* type;
  };

  template<typename _Tp>
    using add_pointer_t = typename add_pointer<_Tp>::type;
}

SYCL_EXTERNAL int foo(int i) {return (int)i+1;}

struct uniform; struct linear; struct varying; struct masked; struct unmasked;
template<int...> struct int_list {};
namespace detail {
template<class...> struct variant_list{};
}

typedef int (*foop)(int);

struct inear{};
struct asked{};

template <typename T>
class my_list { };

void test()
{
  foop fooTable[2];

  // function argument expected
  // expected-error@+1 {{function argument is not valid function}}
  __builtin_generate_SIMD_variant(1,4,
      std::add_pointer_t<unmasked(linear,uniform)>());

  // Is this really legal vlen==0
  // expected-error@+1 {{the vector length '0' must be positive value}}
  __builtin_generate_SIMD_variant(foo, 0,
      std::add_pointer_t<unmasked(linear)>());

  // Ok
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    int_list<4,8>(),
    (foop*)fooTable, 1);

  // Bad first arg
  // expected-error@+2 {{bad variant specifier 'inear', expected 'varying', 'uniform' or 'linear'}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(inear), masked(varying)>(),
    int_list<4,8>(),
    (foop*)fooTable, 1);

  // Bad first arg
  // expected-error@+2 {{bad variant specifier 'asked', expected 'masked' or 'unmasked'}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), asked(varying)>(),
    int_list<4,8>(),
    (foop*)fooTable, 1);

  // Bad second arg
  // expected-error@+3 {{the vector length must be passed as template parameter}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    int_list<>(),
    (foop*)fooTable, 1);

  // Bad second arg
  // expected-error@+3 {{the vector length '-6' must be positive value}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    int_list<-6>(),
    (foop*)fooTable, 1);

  int i = 4;
  // Bad second arg
  // expected-error@+3 {{the vector length must be passed as template parameter}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    my_list<double>(),
    (foop*)fooTable, 1);

  // Bad third arg
  // expected-error@+4 {{invalied function table argument}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    int_list<4,8>(),
    55, 1);

  int *ip = 0;
  // Bad third arg
  // expected-error@+4 {{invalied function table argument}}
  __builtin_call_SIMD_variant(
    detail::variant_list<unmasked(linear), masked(varying)>(),
    int_list<4,8>(),
    ip, 1);
}
