// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only %s -verify -Wno-unused-value

template <unsigned I>
bool check_feature() {
  return _may_i_use_cpu_feature(I);
}

void tests() {
  int i;
  // expected-error@+1 {{argument to '_may_i_use_cpu_feature' must be a constant integer}}
  _may_i_use_cpu_feature(i);
  // expected-error-re@+1 {{cannot initialize a parameter of type 'unsigned {{.*}}long' with an lvalue of type 'const char [8]'}}
 _may_i_use_cpu_feature("FEATURE");

 _may_i_use_cpu_feature(1 | 8);

 check_feature<1 | 8>();

#define FEAT_1 1U << 7
#define FEAT_2 1U << 8
  _may_i_use_cpu_feature(FEAT_1 | FEAT_2);
}
