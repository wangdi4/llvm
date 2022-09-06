// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -Wno-c++20-extensions \
// RUN:  -fsycl-is-device -triple spir64-unknown-unknown-sycldevice \
// RUN:  -no-opaque-pointers %s | FileCheck %s

// RUN: %clang_cc1 -O0 -emit-llvm -o - -std=c++17 -Wno-c++20-extensions \
// RUN:  -no-opaque-pointers -fenable-variant-function-pointers \
// RUN:  -fsycl-is-device -triple spir64-unknown-unknown-sycldevice \
// RUN:  %s | FileCheck %s

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
struct uniform; struct linear; struct varying; struct masked; struct unmasked;
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
        detail::makeSimdFunctionImpl<F, S, T...>());
}

template<auto F, class L, class...T>
constexpr auto makeSimdFunction() noexcept {
    return SimdFunction<std::remove_pointer_t<decltype(F)>, L, T...>(
        detail::makeSimdFunctionImpl<F, T...>(L()));
}

//CHECK: define{{.*}}spir_func noundef i32 @_Z3fooif
SYCL_EXTERNAL int foo(int i, float f) {return (int)f+i+1;}

using namespace cl::sycl::intel;

//CHECK: define{{.*}}_Z3barv
SYCL_EXTERNAL auto bar()
{
  auto foo_simd = makeSimdFunction<foo, int_list<4, 8>,
                                   unmasked(linear, uniform),
                                   masked(varying, varying)>();
  return foo_simd(5,2.0);
}

// Call operator
//CHECK: define{{.*}} noundef i32 {{.*}}_EEEclEOiOf
//CHECK-SAME: class{{.*}}SimdFunction{{.*}}* {{[^,]*}} %this
//CHECK-SAME: i32 [[AS4:addrspace\(4\)]]* noundef align 4 dereferenceable(4) %args
//CHECK-SAME: float [[AS4]]* noundef align 4 dereferenceable(4) %args
//CHECK: [[THISARG:%this.addr.*]] = alloca {{.*}}class{{.*}}SimdFunction{{.*}}*,
//CHECK: [[ARG1:%args.addr.*]] = alloca i32 [[AS4]]*,
//CHECK: [[ARG2:%args.addr.*]] = alloca float [[AS4]]*,
//CHECK: %[[ARG1_CAST:.+]] = addrspacecast i32 [[AS4]]** [[ARG1]] to i32 [[AS4]]* [[AS4]]*
//CHECK: %[[ARG2_CAST:.+]] = addrspacecast float [[AS4]]** [[ARG2]] to float [[AS4]]* [[AS4]]*

//CHECK: %ptrs = getelementptr {{.*}}%this{{.*}} i32 0, i32 0
//CHECK-NEXT: [[C1:%call[0-9]*]] = {{.*}}%ptrs

//CHECK: [[L0:%[0-9]+]] = load i32 [[AS4]]*, i32 [[AS4]]* [[AS4]]* %[[ARG1_CAST]],
//CHECK-NEXT: [[L1:%[0-9]+]] = load i32, i32 [[AS4]]* [[L0]], align 4

//CHECK: [[L2:%[0-9]+]] = load float [[AS4]]*, float [[AS4]]* [[AS4]]* %[[ARG2_CAST]],
//CHECK-NEXT: [[L3:%[0-9]+]] = load float, float [[AS4]]* [[L2]], align 4

//CHECK: @__intel_indirect_call_0
//CHECK-SAME: [[C1]], i32 [[L1]], float [[L3]]) #[[CALL:[0-9]+]]

// makeSimdFunctionImpl
//CHECK: define{{.*}}makeSimdFunctionImpl{{.*}}i4
//CHECK: @{{.*}}@_Z3fooif) #[[UNMASK4:[0-9]+]]
//CHECK: @__intel_create_simd_variant_0{{.*}}@_Z3fooif) #[[MASK4:[0-9]+]]

// makeSimdFunctionImpl
//CHECK: define{{.*}}makeSimdFunctionImpl{{.*}}i8
//CHECK: @__intel_create_simd_variant_0{{.*}}@_Z3fooif) #[[UNMASK8:[0-9]+]]
//CHECK: @__intel_create_simd_variant_0{{.*}}@_Z3fooif) #[[MASK8:[0-9]+]]

//CHECK: attributes #[[CALL]] = { "vector-variants"="_ZGVxN4lu__$U0,_ZGVxM4vv__$U0,_ZGVxN8lu__$U0,_ZGVxM8vv__$U0" }
//CHECK: attributes #[[UNMASK4]] = { "vector-variants"="_ZGVxN4lu__Z3fooif" }
//CHECK: attributes #[[MASK4]] = { "vector-variants"="_ZGVxM4vv__Z3fooif" }
//CHECK: attributes #[[UNMASK8]] = { "vector-variants"="_ZGVxN8lu__Z3fooif" }
//CHECK: attributes #[[MASK8]] = { "vector-variants"="_ZGVxM8vv__Z3fooif" }
