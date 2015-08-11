// CQ#364598
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

struct Cake1 {
  bool is_a_lie();
};

struct Cake2 {
  bool is_a_lie();
};

typedef Cake2 Cake2Typedefed; // expected-note{{declared here}}

class B {
public:
  typedef Cake1 Cake1Typedefed; // expected-note{{declared here}}
  friend struct Cake1Typedefed; // expected-warning{{elaborated befriended type refers to a typedef}}
  friend struct Cake2Typedefed; // expected-error{{elaborated type refers to a typedef}}
};

bool B::Cake1Typedefed::is_a_lie() {
  return true;
}

bool Cake2Typedefed::is_a_lie() {
  return true;
}

int main() {
  B::Cake1Typedefed cake1;
  Cake2Typedefed cake2;
  return cake1.is_a_lie() && cake2.is_a_lie();
}
