// RUN: %clang_cc1 -triple x86_64-linux-pc -fdeclspec -fintel-compatibility -fsyntax-only %s -verify

namespace FreeFuncs {
// expected-error@+1{{'allow_cpu_features' attribute takes at least 1 argument}}
__attribute__((allow_cpu_features())) void first() {}

// expected-error@+1{{'allow_cpu_features' attribute takes no more than 2 arguments}}
__attribute__((allow_cpu_features(1, 2, 3))) void second() {}

__attribute__((allow_cpu_features(2))) void third() {}

__attribute__((allow_cpu_features(2, 2))) void fourth() {}

__declspec(allow_cpu_features(2, 2)) void spelled_declspec() {}
} // namespace FreeFuncs

namespace Templates {
template <typename T>
// expected-error@+1{{'allow_cpu_features' attribute takes at least 1 argument}}
__attribute__((allow_cpu_features())) void first() {}

template <typename T>
// expected-error@+1{{'allow_cpu_features' attribute takes no more than 2 arguments}}
__attribute__((allow_cpu_features(T::value, T::value2, T::value3))) void second() {}

template <typename T>
__attribute__((allow_cpu_features(T::value))) void third() {}

template <typename T>
__attribute__((allow_cpu_features(T::value, T::value2))) void fourth() {}

template <int I>
__attribute__((allow_cpu_features(I))) void fifth() {}

template <int I, int J>
__attribute__((allow_cpu_features(I, J))) void sixth() {}

template <typename T, T V1, typename T2, T2 V2>
struct ValueHolder {
  static constexpr T value = V1;
  static constexpr T2 value2 = V2;
};

struct OtherTy {};

void instantiations() {
  ValueHolder<int, 2, int, 2> VH;

  third<decltype(VH)>();
  fourth<decltype(VH)>();
  fifth<2>();
  sixth<2, 4>();
}
} // namespace Templates

namespace Conflicting {
__attribute__((allow_cpu_features(2))) void f1();
// expected-warning@+2{{attribute 'allow_cpu_features' is already applied with different arguments}}
// expected-note@-2{{previous attribute is here}}
__attribute__((allow_cpu_features(4))) void f1();

__attribute__((allow_cpu_features(2, 2))) void f2();
// expected-warning@+2{{attribute 'allow_cpu_features' is already applied with different arguments}}
// expected-note@-2{{previous attribute is here}}
__attribute__((allow_cpu_features(2))) void f2();

__attribute__((allow_cpu_features(2, 2))) void f3();
// expected-warning@+2{{attribute 'allow_cpu_features' is already applied with different arguments}}
// expected-note@-2{{previous attribute is here}}
__attribute__((allow_cpu_features(2, 4))) void f3();

// expected-warning@+2{{attribute 'allow_cpu_features' is already applied with different arguments}}
// expected-note@+2{{previous attribute is here}}
__attribute__((allow_cpu_features(2, 2)))
__attribute__((allow_cpu_features(2, 4))) void
f4();

template <int I, int J, int K, int L>
__attribute__((allow_cpu_features(I, J))) // #TemplA1
__attribute__((allow_cpu_features(K, L))) void  // #TemplA2
Templ() {}

void use() {
  // No error.
  Templ<2, 2, 2, 2>();

  // expected-warning@#TemplA1{{attribute 'allow_cpu_features' is already applied with different arguments}}
  // expected-note@#TemplA2{{previous attribute is here}}
  // expected-note@+1{{in instantiation of function template specialization}}
  Templ<2, 2, 2, 4>();
}
} // namespace Conflicting

namespace InvalidParams {
// expected-error@+1{{invalid bitmask value specified for 'allow_cpu_features' attribute}}
__attribute__((allow_cpu_features(1ULL << 63))) void invalid_page_1() {}

// expected-error@+1{{invalid bitmask value specified for 'allow_cpu_features' attribute}}
__attribute__((allow_cpu_features(0, 1ULL << 63))) void invalid_page_2() {}

// Page1 is full (with 2 exceptions), so 49 is a good bit here.
__attribute__((allow_cpu_features(1ULL<<49, 0))) void valid_page_1(){}

// Page2 hasn't reached bit 49 yet, so it is an invliad value.
// expected-error@+1{{invalid bitmask value specified for 'allow_cpu_features' attribute}}
__attribute__((allow_cpu_features(0, 1ULL<<49))) void invalid_page_2b(){}

__attribute__((allow_cpu_features(1, 0))) void generic_mask(){}

// '1' (aka 65) is movdiri in page2.
__attribute__((allow_cpu_features(0, 1))) void valid_page_2(){}
} // namespace InvalidParams
