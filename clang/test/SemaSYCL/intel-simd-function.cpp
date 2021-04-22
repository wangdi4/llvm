// RUN: %clang_cc1 -verify -fsyntax-only -std=c++17 -Wno-c++20-extensions \
// RUN:  -fsycl-is-device -triple spir64-unknown-unknown-sycldevice \
// RUN:  %s

namespace std {

  template<typename> struct remove_reference;

  template<typename _Tp> struct remove_const { typedef _Tp type; };
  template<typename _Tp> struct remove_const<_Tp const> { typedef _Tp type; };
  template<typename _Tp> struct remove_volatile { typedef _Tp type; };
  template<typename _Tp>
  struct remove_volatile<_Tp volatile> { typedef _Tp type; };

  template<typename _Tp>
  struct remove_cv {
      typedef typename
      remove_const<typename remove_volatile<_Tp>::type>::type type;
  };

  template<typename _Tp, typename>
  struct __remove_pointer_helper { typedef _Tp type; };

  template<typename _Tp, typename _Up>
  struct __remove_pointer_helper<_Tp, _Up*> { typedef _Up type; };

  template<typename _Tp>
  struct remove_pointer
    : public __remove_pointer_helper<_Tp, typename remove_cv<_Tp>::type>
    { };

  template<typename _Tp>
  struct add_pointer {
    typedef typename remove_reference<_Tp>::type* type;
  };

  template<typename _Tp>
    using remove_pointer_t = typename remove_pointer<_Tp>::type;

  template<typename _Tp>
    using add_pointer_t = typename add_pointer<_Tp>::type;

  template<typename _Tp> struct remove_reference { typedef _Tp type; };

  template<typename _Tp, _Tp __v>
  struct integral_constant
  {
      static constexpr _Tp value = __v;
      typedef _Tp value_type;
      typedef integral_constant<_Tp, __v> type;
      constexpr operator value_type() const noexcept { return value; }
      constexpr value_type operator()() const noexcept { return value; }
  };
  typedef integral_constant<bool, true> true_type;
  typedef integral_constant<bool, false> false_type;


  template<bool, typename _Tp = void> struct enable_if { };

  template<typename _Tp> struct enable_if<true, _Tp> { typedef _Tp type; };

  template<bool _Cond, typename _Tp = void>
  using enable_if_t = typename enable_if<_Cond, _Tp>::type;

  template<typename _Tp>
  constexpr _Tp&&
  forward(typename std::remove_reference<_Tp>::type& __t) noexcept
  { return static_cast<_Tp&&>(__t); }

  typedef decltype(nullptr) nullptr_t;
  typedef long unsigned int size_t;
  template<typename _Tp, std::size_t _Nm>
  struct array { typedef _Tp* pointer;
                 typedef const _Tp* const_pointer;
                 typedef _Tp* iterator;
                 typedef const _Tp* const_iterator;
                 _Tp _M_elems[_Nm];
                 pointer data() noexcept { return &_M_elems[0]; }
                 const_pointer data() const noexcept { return &_M_elems[0]; }
                 iterator begin() noexcept { return iterator(data()); }
                 const_iterator begin() const noexcept {
                   return const_iterator(data()); }
                 iterator end() noexcept { return iterator(data() + _Nm); }
                 const_iterator end() const noexcept {
                   return const_iterator(data() + _Nm); }
  };

  template<typename, typename>
  struct is_same : public false_type { };

  template<typename _Tp>
  struct is_same<_Tp, _Tp> : public true_type { };

  template< class T, class U >
  inline constexpr bool is_same_v = is_same<T, U>::value;


  template<typename _Tp, typename... _Up>
    array(_Tp, _Up...)
      -> array<enable_if_t<(is_same_v<_Tp, _Up> && ...), _Tp>,
        1 + sizeof...(_Up)>;

  template<typename _II, typename _OI>
  _OI copy(_II __first, _II __last, _OI __result) { return __result; }

}

#define FWD(x) std::forward<decltype(x)>(x)

namespace cl { namespace sycl { namespace intel {
// expected-note@+1 {{declared here}}
struct uniform; struct linear; struct varying; struct masked; struct unmasked;
struct xxxx;
}}}

template<int...>
struct int_list {};

namespace detail {
  template<class...> struct variant_list{};

//return index of first matching type
template<class T, class U, class...Us>
static constexpr int find() {
    if constexpr (std::is_same_v<T, U>) return 0;
    else return find<T, Us...>() + 1;
}
//return index of first matching value
template<auto T, auto U, auto...Us>
static constexpr int find() {
    if constexpr (T==U) return 0;
    else return find<T, Us...>() + 1;
}
//return whether any type matches
template <class T, class... U>
constexpr bool contains_type_v = (std::is_same_v<T, U> || ...);
//return whether any value matches
template<auto T, auto...U> constexpr bool contains_value_v = ((T==U) || ...);

template<auto F, int S, class...T>
constexpr auto makeSimdFunctionImpl() noexcept {
  return std::array{
  // expected-error@+7 {{bad variant specifier 'cl::sycl::intel::linear (*)(int)', expected 'varying', 'uniform' or 'linear'}}
  // expected-error@+6 {{bad variant specifier 'int', expected 'masked' or 'unmasked'}}
  // expected-error@+5 {{the vector length '-4' must be positive value}}
  // expected-error@+4 {{the vector length '-8' must be positive value}}
  // expected-error@+3 {{vector length '-16' must be positive value}}
  // expected-error@+2 {{bad variant specifier , expected variant_list}}
  // expected-error@+1 {{bad variant specifier 'linear', expected 'masked' or 'unmasked'}}
          __builtin_generate_SIMD_variant(F, S, std::add_pointer_t<T>())...};
}
template<class T, std::size_t N, std::size_t M>
constexpr auto flatten(const std::array<std::array<T, N>, M>& c) {
  std::array<T, N*M> a;
  for (auto it = a.begin(); auto& e: c) it = std::copy(e.begin(), e.end(), it);
  return a;
}

template<auto F, class...T, int...S>
constexpr auto makeSimdFunctionImpl(int_list<S...>) noexcept {
    // expected-note@+2 {{requested here}}
    // expected-note@+1 {{requested here}}
    return flatten(std::array{makeSimdFunctionImpl<F, S, T...>()...});
}

} // namespace detail

//SIMD function wrapper
template<typename F, class L, typename...T> class SimdFunction;

template<typename Ret, int...S, class...Args, typename...T>
class SimdFunction <Ret(Args...), int_list<S...>, T...> {
    using F = Ret(Args...);
    template<int I>
    using int_constant = std::integral_constant<int, I>;
public:
    //Copy+Move constructor/assignment & destructor are all implicit default
    //Default constructor
    constexpr SimdFunction() = default;
    constexpr SimdFunction(std::nullptr_t) noexcept : ptrs{} {}
    //Conversion constructor. Enabled if source contains all required variants
    //  and sg-sizes
    template <typename... OT, int... OS,
              typename = std::enable_if_t<
                  (detail::contains_type_v<T, OT...> && ...)>,
              typename = std::enable_if_t<
                  (detail::contains_value_v<S, OS...> && ...)>>
    constexpr
    SimdFunction(const SimdFunction<F, int_list<OS...>, OT...> &o) noexcept {
      constexpr std::array va{detail::find<T, OT...>()...};
      constexpr std::array sa{detail::find<S, OS...>()...};
      for (int i = 0; auto s : sa)
        for (auto v : va)
          ptrs[i++] = o.ptrs[v + s * va.size()];
    }
    //Call operator·
    Ret operator()(Args&&...args) const {
      //expected-error@+2 {{wrong number of parameters in variant specifier, expected '1', have '2'}}
      return __builtin_call_SIMD_variant(
          detail::variant_list<T...>(), int_list<S...>(), ptrs.data(),
          FWD(args)...);
    }
    //Conversion to bool. Class invariant that either none or all ptrs are null.
    constexpr explicit operator bool() const noexcept { return ptrs[0];}

private:
    constexpr SimdFunction(std::array<F *, sizeof...(T) * sizeof...(S)> p)
        : ptrs(p) {}
    template <auto, int, class...>
    friend constexpr auto makeSimdFunction() noexcept;
    template <auto, class, class...>
    friend constexpr auto makeSimdFunction() noexcept;
    std::array<F *, sizeof...(T) * sizeof...(S)> ptrs;
    template <class F, class L, class... O>
    friend class SimdFunction;
};

template<auto F, int S, class...T>
constexpr auto makeSimdFunction() noexcept {
    return SimdFunction<std::remove_pointer_t<decltype(F)>, int_list<S>, T...>(
       // expected-note@+5 {{requested here}}
       // expected-note@+4 {{requested here}}
       // expected-note@+3 {{requested here}}
       // expected-note@+2 {{requested here}}
       // expected-note@+1 {{requested here}}
        detail::makeSimdFunctionImpl<F, S, T...>());
}

template<auto F, class L, class...T>
constexpr auto makeSimdFunction() noexcept {
    return SimdFunction<std::remove_pointer_t<decltype(F)>, L, T...>(
        // expected-note@+2 {{requested here}}
        // expected-note@+1 {{requested here}}
        detail::makeSimdFunctionImpl<F, T...>(L()));
}

SYCL_EXTERNAL int foo(int i) {return (int)i+1;}

using namespace cl::sycl::intel;


/*SYCL_EXTERNAL*/ auto error_cases()
{
  // expected-error@+1 {{'linear' does not refer to a value}}
  auto fs1 =  makeSimdFunction<foo, 8, other(linear)>();
  // expected-error@+1 {{use of undeclared identifier 'other'}}
  auto fs2 =  makeSimdFunction<foo, 8, masked(other)>();
  // expected-note@+1 {{requested here}}
  auto fs3 =  makeSimdFunction<foo, 8, linear(masked)>();
  // expected-note@+1 {{requested here}}
  auto fs4 =  makeSimdFunction<foo, 8, int>();
  auto fs5 =  makeSimdFunction<foo, 8, masked(linear, linear)>();
  // expected-note@+1 {{requested here}}
  auto fs6 =  makeSimdFunction<foo, -4, masked(linear)>();
  // expected-note@+1 {{requested here}}
  auto fs7 =  makeSimdFunction<foo, int_list<8, -8>, masked(linear)>();
  // expected-note@+1 {{requested here}}
  auto fs8 = makeSimdFunction<foo, int_list<-16, 8>, masked(linear)>();
  // expected-note@+1 {{requested here}}
  auto fs9 = makeSimdFunction<foo, 4, int(masked)>();
  // expected-note@+1 {{requested here}}
  auto fs10 = makeSimdFunction<foo, 8, masked(linear(int))>();
  // expected-note@+1 {{requested here}}
  fs5(0);
}

template<typename T>
SYCL_EXTERNAL auto error_cases1(T value) {
  typedef int (*fp)(int);
  // expected-error@+1 {{vector length '-1' must be positive value}}
  __builtin_generate_SIMD_variant(foo, (int)(sizeof(sizeof(value))) - 9,
                                  std::add_pointer_t<unmasked(linear)>());
  // expected-error@+2 {{bad variant specifier 'xxxx', expected 'varying', 'uniform' or 'linear'}}
  __builtin_generate_SIMD_variant(foo, 4,
                                  std::add_pointer_t<unmasked(xxxx)>());
  // expected-error@+1 {{must be a constant integer}}
  __builtin_generate_SIMD_variant(foo, int_list<1,5>(),
                                  std::add_pointer_t<unmasked(linear)>());
  auto ptrs = (fp*)__builtin_generate_SIMD_variant(foo, 4,
                                  std::add_pointer_t<unmasked(linear)>());
  __builtin_call_SIMD_variant(detail::variant_list<unmasked(linear),
  // expected-error@+1 {{the vector length must be passed as template parameter}}
                               masked(varying)>(), 4, ptrs, 1);
}

template<class ...Ts>
SYCL_EXTERNAL auto bar(Ts ...Args)
{
   __builtin_generate_SIMD_variant(&Args...); //expected-no-error
}
template<typename T> int zoo(T value)
{
  bar(foo, value, std::add_pointer_t<unmasked(linear,uniform)>());
}
void yoo() {
  zoo<int>(1);
}
