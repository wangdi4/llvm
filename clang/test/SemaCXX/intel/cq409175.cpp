// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s

namespace NS {
  struct S {int x,y;};
}

struct NS::S; // expected-warning {{forward declaration of struct cannot have a nested name specifier}}
NS::S s = {1,2}; 
