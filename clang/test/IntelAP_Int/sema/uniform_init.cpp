// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -fsyntax-only -verify -pedantic -Wconversion -Wall -Wextra -Wpedantic

// Tests whether we can use uniform initialization to initialize one ap_int for
// another with different widths.

template <int Bits>
using ap_int = int __attribute__((__ap_int(Bits)));

int main() {
  ap_int<33> val33 = 1;
  // expected-warning@+1{{implicit conversion loses integer precision}}
  ap_int<17> val17_a = val33;
  // expected-warning@+1{{implicit conversion loses integer precision}}
  ap_int<17> val17_b(val33);
  // expected-error@+2{{non-constant-expression cannot be narrowed from type}}
  // expected-warning@+1{{implicit conversion loses integer precision}}
  ap_int<17> val17_c{val33};
  (void)val33;
  (void)val17_a;
  (void)val17_b;
  (void)val17_c;
}

struct my_int33 {
  ap_int<33> val33;
};
struct my_int17 {
  ap_int<17> val17;
  // expected-error@+2{{non-constant-expression cannot be narrowed from type}}
  // expected-warning@+1{{implicit conversion loses integer precision}}
  my_int17(my_int33 i) : val17{i.val33} {}
  // expected-warning@+1{{implicit conversion loses integer precision}}
  my_int17 (my_int33 i, bool) : val17(i.val33) {}
};

