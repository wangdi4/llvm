// RUN: %clang_cc1 -fsyntax-only -Wc++11-narrowing -fintel-compatibility -verify %s

struct VirtualBase {
  VirtualBase() {}
  virtual void foo() {}
};

struct Mike : virtual VirtualBase {
  Mike(int...);
  virtual void foo() {}
};

Mike::Mike(int i, ...) {
  // expected-error@+1 {{cannot initialize static variable with label address}}
  static void *ouch = &&x;
  goto *ouch;
x:;
}

struct Dave : virtual Mike {
  Dave() : Mike(2, 3, 4, 5) {}
  void foo() {}
};

int main() {
  /* complete object construction */
  Mike *m = new Mike(1);
  /* construction of Mike sub object */
  VirtualBase *v = new Dave;
  Mike *m2 = new Mike(1);

  return 0;
}

