// CQ#367740
// RUN: %clang_cc1 -fsyntax-only -verify %s -DERROR
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s -DWARNING

#if ERROR

#pragma message disable missingreturn // expected-error{{pragma message requires parenthesized string}}
int main() {
  return 0;
}

#elif WARNING

#pragma message disable missingreturn // expected-warning{{pragma message requires parenthesized string}}
int main() {
    return 0;
}

#else

#error Unknown test mode

#endif
