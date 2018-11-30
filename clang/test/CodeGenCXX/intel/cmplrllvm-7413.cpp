//RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 \
//RUN:  -std=c++11 -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-MS
//RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu \
//RUN:  -std=c++11 -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-LX

// This tests that programs that rely on substitution failure of static_cast
// work as expected.  Similar usage was found in range-v3.

// Usual types
struct true_type { static constexpr bool value = true; };
struct false_type { static constexpr bool value = false; };
template <bool C> struct enable_if { };
template <> struct enable_if<true> { typedef void type; };
template <typename ...>
struct void_t_impl { typedef void type; };
template <typename ... T>
using void_t = typename void_t_impl<T...>::type;
template <typename T>
T declval() noexcept { static_assert(sizeof(T) == 0, "foo");}

// type trait to test static_cast
template <typename From, typename To, typename = void>
struct is_good : false_type {};

template <typename From, typename To>
struct is_good<From, To, void_t< decltype(static_cast<To>(declval<From>()))>>
: true_type {};

// overload based on the type trait
template <typename From, typename To,
          typename = typename enable_if<is_good<From, To>::value>::type>
int bar(From f, To) {
  To t1 = static_cast<To>(f);
  return 1;
}

int bar(const void *v1, const void *v2) {
  return 2;
}
// CHECK-LX: define{{.*}}main
// CHECK-MS: define{{.*}}main
int main() {
  int fails = 0;
  char c = ' ';
  //CHECK-LX: call{{.*}}_Z3barIPcS0_vEiT_T0_
  //CHECK-MS: call{{.*}}??$bar@PEADPEADX@@YAHPEAD0@Z
  if (bar(&c, &c) != 1) fails++;
  //CHECK-LX: call{{.*}}_Z3barPKvS0_
  //CHECK-MS: call{{.*}}?bar@@YAHPEBX0@Z
  if (bar("aaa", &c) != 2) fails++;
  return fails;
}
