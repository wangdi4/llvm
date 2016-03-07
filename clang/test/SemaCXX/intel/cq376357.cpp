// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive -triple x86_64-unknown-linux-gnu %s

int f1(void* p) {
  return reinterpret_cast<int>(p);
}

template<class T1, class T2>
T1 f2(T2 p) {
  return reinterpret_cast<T1>(p);
}
short f2_test(void* p) {
  return f2<short>(p);
}

void f3(void) {
  int i;
  int* pi = &i;
  int** ppi = &pi;
  void** vpp = ppi;
  // expected-warning@-1{{illegal implicit type conversion from 'int **' to 'void **' allowed in -fpermissive mode}}
  vpp = &ppi;
  // expected-warning@-1{{illegal implicit type conversion from 'int ***' to 'void **' allowed in -fpermissive mode}}
}

template<class T1, class T2>
void f3() {
  T1 i;
  T1* pi = &i;
  T1** ppi = &pi;
  T2** vpp = ppi;
  // expected-warning@-1{{illegal implicit type conversion from 'short **' to 'void **' allowed in -fpermissive mode}}
  vpp = &ppi;
  // expected-warning@-1{{illegal implicit type conversion from 'short ***' to 'void **' allowed in -fpermissive mode}}
}
void f3_test() {
  f3<short, void>();
  // expected-note@-1{{in instantiation of function template specialization 'f3<short, void>' requested here}}
}

int* f4(void* pv) {
  return pv;
  // expected-warning@-1{{illegal implicit type conversion from 'void *' to 'int *' allowed in -fpermissive mode}}
}

template<class T1, class T2>
T1 fx(T2 t) {
  T1 a = t; // initialization
  // expected-warning@-1{{illegal implicit type conversion from 'void *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-2{{illegal implicit type conversion from 'long *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-3{{illegal implicit type conversion from 'unsigned int *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-4{{illegal implicit type conversion from 'int (**)(char, char)' to 'void (**)(int)' allowed in -fpermissive mode}}
  // expected-warning@-5{{illegal implicit type conversion from 'int (*)(char, char)' to 'void (*)(int)' allowed in -fpermissive mode}}
  // expected-warning@-6{{illegal implicit type conversion from 'short *' to 'char' allowed in -fpermissive mode}}
  // expected-warning@-7{{illegal implicit type conversion from 'short' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-8{{illegal implicit type conversion from 'int' to 'E' allowed in -fpermissive mode}}
  a = t; // assignment
  // expected-warning@-1{{illegal implicit type conversion from 'void *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-2{{illegal implicit type conversion from 'long *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-3{{illegal implicit type conversion from 'unsigned int *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-4{{illegal implicit type conversion from 'int (**)(char, char)' to 'void (**)(int)' allowed in -fpermissive mode}}
  // expected-warning@-5{{illegal implicit type conversion from 'int (*)(char, char)' to 'void (*)(int)' allowed in -fpermissive mode}}
  // expected-warning@-6{{illegal implicit type conversion from 'short *' to 'char' allowed in -fpermissive mode}}
  // expected-warning@-7{{illegal implicit type conversion from 'short' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-8{{illegal implicit type conversion from 'int' to 'E' allowed in -fpermissive mode}}
  void foo(T1);
  foo(t); // pass as a parameter
  // expected-warning@-1{{illegal implicit type conversion from 'void *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-2{{illegal implicit type conversion from 'long *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-3{{illegal implicit type conversion from 'unsigned int *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-4{{illegal implicit type conversion from 'int (**)(char, char)' to 'void (**)(int)' allowed in -fpermissive mode}}
  // expected-warning@-5{{illegal implicit type conversion from 'int (*)(char, char)' to 'void (*)(int)' allowed in -fpermissive mode}}
  // expected-warning@-6{{illegal implicit type conversion from 'short *' to 'char' allowed in -fpermissive mode}}
  // expected-warning@-7{{illegal implicit type conversion from 'short' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-8{{illegal implicit type conversion from 'int' to 'E' allowed in -fpermissive mode}}
  return t; // return by value
  // expected-warning@-1{{illegal implicit type conversion from 'void *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-2{{illegal implicit type conversion from 'long *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-3{{illegal implicit type conversion from 'unsigned int *' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-4{{illegal implicit type conversion from 'int (**)(char, char)' to 'void (**)(int)' allowed in -fpermissive mode}}
  // expected-warning@-5{{illegal implicit type conversion from 'int (*)(char, char)' to 'void (*)(int)' allowed in -fpermissive mode}}
  // expected-warning@-6{{illegal implicit type conversion from 'short *' to 'char' allowed in -fpermissive mode}}
  // expected-warning@-7{{illegal implicit type conversion from 'short' to 'int *' allowed in -fpermissive mode}}
  // expected-warning@-8{{illegal implicit type conversion from 'int' to 'E' allowed in -fpermissive mode}}
}
int* f4_test(void* pv) {
  return fx<int*, void*>(pv);
  // expected-note@-1{{in instantiation of function template specialization 'fx<int *, void *>' requested here}}
}

int* f5(unsigned* pu) {
  return pu;
  // expected-warning@-1{{illegal implicit type conversion from 'unsigned int *' to 'int *' allowed in -fpermissive mode}}
}
int* f5_test(unsigned* pu) {
  fx<int*, long*>(0); // ICC only
  // expected-note@-1{{in instantiation of function template specialization 'fx<int *, long *>' requested here}}
  return fx<int*, unsigned*>(pu);
  // expected-note@-1{{in instantiation of function template specialization 'fx<int *, unsigned int *>' requested here}}
}

typedef void (*pf1_t)(int);
typedef int (*pf2_t)(char, char);
pf1_t f6(pf2_t p) {
  return p;
  // expected-warning@-1{{illegal implicit type conversion from 'pf2_t' (aka 'int (*)(char, char)') to 'pf1_t' (aka 'void (*)(int)') allowed in -fpermissive mode}}
}
pf1_t f6_test(pf2_t p) {
  fx<pf1_t*, pf2_t*>(0);
  // expected-note@-1{{in instantiation of function template specialization 'fx<void (**)(int), int (**)(char, char)>' requested here}}
  return fx<pf1_t, pf2_t>(p);
  // expected-note@-1{{in instantiation of function template specialization 'fx<void (*)(int), int (*)(char, char)>' requested here}}
}

int* f7(int i) {
  return i;
  // expected-warning@-1{{illegal implicit type conversion from 'int' to 'int *' allowed in -fpermissive mode}}
}
int* f7_test(int i) {
  fx<char, short*>(0);
  // expected-note@-1{{in instantiation of function template specialization 'fx<char, short *>' requested here}}
  return fx<int*, short>(i);
  // expected-note@-1{{in instantiation of function template specialization 'fx<int *, short>' requested here}}
}

enum E {
  e0, e1
};
int* f8(int) {
  char* p = e1;
  // expected-warning@-1{{illegal implicit type conversion from 'E' to 'char *' allowed in -fpermissive mode}}
  return p + e1;
  // expected-warning@-1{{illegal implicit type conversion from 'char *' to 'int *' allowed in -fpermissive mode}}
}

struct A {
  A(char);
};

void foo(short*, int);
void foo(char**, short*);
void foo(A, int);
int foo(...);

int f9(long a, short* p)
{
  return foo(a, p); // ellipsis function should be called here
}

E f10(int i) {
  return i;
  // expected-warning@-1{{illegal implicit type conversion from 'int' to 'E' allowed in -fpermissive mode}}
}
E f10_test(int i) {
  return fx<E, int>(i);
  // expected-note@-1{{in instantiation of function template specialization 'fx<E, int>' requested here}}
}
