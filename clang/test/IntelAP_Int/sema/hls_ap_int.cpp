// Checks correctly parsed, but invalid ap_int/ap_uint declarations
// Similar to clang/test/SemaCXX/ap_int.cpp test in trunk

// RUN: %clang -cc1 -O3 -disable-llvm-passes -fhls %s -fsyntax-only -verify -pedantic -Wconversion -Wall -Wextra -Wpedantic

int foo() {
  // expected-error@+1{{invalid __ap_int element type 'float'}}
  typedef float __attribute__((__ap_int(5))) WrongType;
  // expected-error@+1{{invalid __ap_int element type 'short'}}
  typedef short __attribute__((__ap_int(5))) WrongType2;
  // expected-error@+1{{__ap_int attribute requires an integer constant}}
  typedef int __attribute__((__ap_int(5.0))) MustBeConstInt;

  // expected-error@+1{{signed __ap_int must have a size of at least 2}}
  typedef int __attribute__((__ap_int(0))) ZeroSize;
  // expected-error@+1{{unsigned __ap_int must have a size of at least 1}}
  typedef unsigned int __attribute__((__ap_int(0))) ZeroSizeUnsigned;
  // expected-error@+1{{signed __ap_int must have a size of at least 2}}
  typedef int __attribute__((__ap_int(1))) SignedOneSize;
  // This is OK.
  typedef unsigned int __attribute__((__ap_int(1))) UnsignedOneSize;
  // expected-error@+1{{signed __ap_int must have a size of at least 2}}
  typedef int __attribute__((__ap_int(-1))) NegSize;
  // expected-error@+1{{unsigned __ap_int must have a size of at least 1}}
  typedef unsigned int __attribute__((__ap_int(-1))) NegSizeUnsigned;

  typedef unsigned int __attribute__((__ap_int(1))) uint1_tt;
  typedef int __attribute__((__ap_int(3))) int3_tt;
  typedef unsigned int __attribute__((__ap_int(3))) uint3_tt;
  typedef unsigned __attribute__((__ap_int(4))) uint4_tt;

  // expected-warning@+1{{implicit conversion from 'int' to 'int3_tt' (aka '__ap_int(3) int') changes value from 9 to 1}}
  int3_tt a = 9;
  // expected-warning@+1{{implicit conversion changes signedness: 'int3_tt' (aka '__ap_int(3) int') to 'uint3_tt' (aka '__ap_int(3) unsigned int'}}
  uint3_tt b = a;
  // expected-warning@+1{{implicit conversion from 'uint3_tt' (aka '__ap_int(3) unsigned int') to 'uint1_tt' (aka '__ap_int(1) unsigned int') changes value from 2 to 0}}
  uint1_tt c = (uint3_tt)2;
  return 0;
}

template<int bits> using ap_int = int __attribute__((__ap_int(bits)));
template<unsigned int bits> using ap_uint = unsigned int
__attribute__((__ap_int(bits)));

bool unknown_b();
void ternary_thing() {
  bool cond = unknown_b();
  ap_uint<6> count = 6;
  ap_int<33> zero = 0;
  ap_int<31> zero_1 = 0;
  int zero_2 = 0;

  count = cond ? count + 1 : zero; // expected-warning{{implicit conversion loses integer precision}}
  count = cond ? count + 1 : zero_1; // expected-warning{{implicit conversion loses integer precision}}
  count = cond ? count + 1 : zero_2; // expected-warning{{implicit conversion loses integer precision}}
}

void sizeof_test() {
  ap_int<56> a;
  ap_int<56> as[2];
  ap_int<66> a2;
  ap_int<66> a2s[2];

  static_assert(sizeof(a) == 8, "Lie about sizeof so that array allocs work");
  static_assert(sizeof(as) == 16, "Lie about sizeof so that array allocs work");
  static_assert(sizeof(a2) == 16, "Lie about sizeof so that array allocs work");
  static_assert(sizeof(a2s) == 32, "Lie about sizeof so that array allocs work");
}
