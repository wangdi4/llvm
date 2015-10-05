// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

// expected-note@+1 {{candidate constructor (the implicit copy constructor) not viable: no known conversion from 'int' to 'const Base' for 1st argument}}
struct Base {
  // expected-note@+1 {{candidate constructor not viable: no known conversion from 'int' to 'int *' for 1st argument; take the address of the argument with &}}
  Base(int *) {}
  virtual ~Base() {}
};

template <typename T>
struct Derived :public Base {
  // expected-error@+1 {{no matching constructor for initialization of 'Base'}}
  Derived(int a) : Base(a) {}
  virtual ~Derived() {}
};

int main(int argc, char **) {
  // expected-note@+1 {{in instantiation of member function 'Derived<int>::Derived' requested here}}
  Derived<int> a(argc);
  return 0;
}
