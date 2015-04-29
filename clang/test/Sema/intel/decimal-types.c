// CQ#369184
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

typedef float __decfloat32 __attribute__((mode(SD))); // expected-error {{GNU decimal type extension not supported}}
typedef float __decfloat64 __attribute__((mode(DD))); // expected-error {{GNU decimal type extension not supported}}
typedef float __decfloat128 __attribute__((mode(TD))); // expected-error {{GNU decimal type extension not supported}}

int main() {
  float a = 2.0dl; // expected-error {{GNU decimal type extension not supported}}
  float b = 2.0DL; // expected-error {{GNU decimal type extension not supported}}
  float c = 2.0df; // expected-error {{GNU decimal type extension not supported}}
  float e = 2.0DF; // expected-error {{GNU decimal type extension not supported}}
  float f = 2.0dd; // expected-error {{GNU decimal type extension not supported}}
  float g = 2.0DD; // expected-error {{GNU decimal type extension not supported}}
  return 0;
}

