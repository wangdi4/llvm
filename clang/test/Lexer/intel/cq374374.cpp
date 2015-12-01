// RUN: %clang_cc1 -std=c++11 -fintel-compatibility -verify %s

#define A "a"

int main()
{
  char c[2] = ""A; // expected-warning{{invalid suffix on literal; C++11 requires a space between literal and identifier}}
  return 0;
}
