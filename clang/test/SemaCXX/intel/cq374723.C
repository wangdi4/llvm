// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility  -O0 -g -ansi -std=c++0x  -verify %s
// expected-no-diagnostics

struct F
{
      template <typename = int>
                void bar ();
};
template <typename = int>
struct V
{
      V (const V &) { F::bar <>; }
};

