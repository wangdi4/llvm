// RUN: %clang_cc1 -std=c++17 -fblocks -DSHOW_MS -Wno-unused-value \
// RUN: -fms-compatibility -fdelayed-template-parsing -fsyntax-only -verify %s
// Temporarily add this file to intel/ pending pulldown from llorg
// expected Feb 2021.

template <typename RT, typename ET> void Decider(const RT &sp, ET &ep) {
  [=](auto i) { ep[i] = sp[i + j]; };
  // expected-error@-1 {{use of undeclared identifier 'j'}}
}

template <typename EMT> void LS() {
  int *ep;
  Decider(5, ep);
}

void runChapter4() {
  LS<int>(); // expected-note {{in instantiation of}}
}
