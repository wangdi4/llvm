// CQ#373129
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

template<typename T1, typename T2, int N>
struct AssumeAlignedCorrect {
  T1 a;
  T2 b;

  void check() {
    __assume_aligned(a, 32);
    __assume_aligned(a, N);
    __assume_aligned(b, 1);
  }
};

template<typename T1, typename T2, int N>
struct AssumeAlignedIncorrect {
  T1 a;
  T2 b;

  void check() {
    __assume_aligned(a); // expected-error {{too few arguments to function call, expected 2, have 1}}
    __assume_aligned(a, 32, N); // expected-error {{too many arguments to function call, expected 2, have 3}}
    __assume_aligned(N, N); // expected-error {{first argument to '__assume_aligned' must be a pointer}}
    __assume_aligned(b, N); // expected-error {{first argument to '__assume_aligned' must be a pointer}}
    __assume_aligned(a, 2 * N + 1); // expected-error {{requested alignment is not a power of 2}}
  }
};

void test() {
  // Tests for correct cases.
  AssumeAlignedCorrect<int *, char *, 32> correct;
  correct.check();

  // Tests for incorrect cases.
  AssumeAlignedIncorrect<int *, char, 64> incorrect;
  incorrect.check(); // expected-note{{in instantiation of member function 'AssumeAlignedIncorrect<int *, char, 64>::check' requested here}}
}
