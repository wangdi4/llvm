// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -Wno-unused-value

#define FEAT_1 1U << 7
#define FEAT_2 1U << 8

template <unsigned I>
bool check_feature() {
  return _may_i_use_cpu_feature(I);
}

template <unsigned I, unsigned J>
bool check_feature_ext() {
  return _may_i_use_cpu_feature_ext(I, J);
}

template<typename T, typename U>
struct is_same {
  static constexpr bool value = false;
};
template<typename T>
struct is_same<T,T> {
  static constexpr bool value = true;
};

void tests() {
  int i;
  // expected-error@+1 {{argument to '_may_i_use_cpu_feature' must be a constant integer}}
  _may_i_use_cpu_feature(i);
  // expected-error-re@+1 {{cannot initialize a parameter of type 'unsigned {{.*}}long' with an lvalue of type 'const char [8]'}}
 _may_i_use_cpu_feature("FEATURE");

 _may_i_use_cpu_feature(1 | 8);

 check_feature<1 | 8>();
 _may_i_use_cpu_feature(FEAT_1 | FEAT_2);
 static_assert(is_same<int, decltype(_may_i_use_cpu_feature(0))>::value, "should return int");
}

void tests2() {
  int i;
  // expected-error@+1 {{argument to '_may_i_use_cpu_feature_ext' must be a constant integer}}
  _may_i_use_cpu_feature_ext(i, 0);
  // expected-error-re@+1 {{cannot initialize a parameter of type 'unsigned {{.*}}long' with an lvalue of type 'const char [8]'}}
  _may_i_use_cpu_feature_ext("FEATURE", 0);

  // expected-error@+1 {{argument to '_may_i_use_cpu_feature_ext' must be a constant integer}}
  _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, i);
  // expected-error@+1 {{cannot initialize a parameter of type 'unsigned int' with an lvalue of type 'const char [8]'}}
  _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, "FEATURE");
  // expected-error@+1 {{argument value 2 is outside the valid range [0, 1]}}
  _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 2);

  check_feature_ext<1 | 8, 1>();
  _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 0);
  _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 1);
  static_assert(is_same<int, decltype(_may_i_use_cpu_feature_ext(0, 0))>::value, "should return int");
}

void test3() {
  _may_i_use_cpu_feature_str(0);          // expected-error{{expression is not a string literal}}
  _may_i_use_cpu_feature_str("avx2", 0);  // expected-error{{expression is not a string literal}}
  _may_i_use_cpu_feature_str("NONSENSE"); // expected-error{{invalid cpu feature string for builtin}}
  _may_i_use_cpu_feature_str("avx2", " bmi");
  _may_i_use_cpu_feature_str("avx2", "bmi");
  _may_i_use_cpu_feature_str("bad", "avx2", "bmi"); // expected-error{{invalid cpu feature string for builtin}}
  _may_i_use_cpu_feature_str("avx2", "bad", "bmi"); // expected-error{{invalid cpu feature string for builtin}}
  _may_i_use_cpu_feature_str("avx2", "bmi", "bad"); // expected-error{{invalid cpu feature string for builtin}}

  // check amx-tile, amx-int8, amx-bf16, should not throw error.
  _may_i_use_cpu_feature_str("amx-tile", "amx-bf16", "amx-int8");

  // check avxvnni key-locker, wide-kl should not throw error.
  _may_i_use_cpu_feature_str("avxvnni", "kl", "widekl");
}
