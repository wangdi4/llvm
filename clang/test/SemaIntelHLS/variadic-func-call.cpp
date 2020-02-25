// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic -triple spir64-unknown-unknown-intelfpga %s

void variadic(int, ...);
namespace NS {
void variadic(int, ...);
}

struct S {
  S(int, ...);
  void operator()(int, ...);
};

void foo() {
  auto x = [](int, ...) {};
  x(5, 10); //expected-error{{variadic function cannot use spir_function calling convention}}
}

void overloaded(int, int);
void overloaded(int, ...);
template <typename, typename Func>
__attribute__((hls_device)) void task(Func KF) {
  KF(); // expected-note 2 {{called by 'task}}
}

int main() {
  task<class FK>([]() {
    variadic(5);        //expected-error{{variadic function cannot use spir_function calling convention}}
    variadic(5, 2);     //expected-error{{variadic function cannot use spir_function calling convention}}
    NS::variadic(5, 3); //expected-error{{variadic function cannot use spir_function calling convention}}
    S s(5, 4);          //expected-error{{variadic function cannot use spir_function calling convention}}
    S s2(5);            //expected-error{{variadic function cannot use spir_function calling convention}}
    s(5, 5);            //expected-error{{variadic function cannot use spir_function calling convention}}
    s2(5);              //expected-error{{variadic function cannot use spir_function calling convention}}
    foo();              //expected-note{{called by 'operator()'}}
    overloaded(5, 6);   //expected-no-error
    overloaded(5, s);   //expected-error{{variadic function cannot use spir_function calling convention}}
    overloaded(5);      //expected-error{{variadic function cannot use spir_function calling convention}}
  });
}

