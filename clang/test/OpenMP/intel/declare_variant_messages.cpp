//RUN: %clang_cc1 -triple=x86_64-pc-win32 -verify -fopenmp \
//RUN:   -fopenmp-late-outline -x c++ -std=c++11 -fms-extensions \
//RUN:   -Wno-pragma-pack %s

void foo(int);
// expected-error@+1 {{expected an OpenMP directive}}
#pragma omp declare

// expected-error@+2 {{'#pragma omp declare variant' can only be applied to functions}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
int a;
// expected-error@+2 {{'#pragma omp declare variant' can only be applied to functions}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma omp threadprivate(a)
int var;
#pragma omp threadprivate(var)

// expected-error@+2 {{expected an OpenMP directive}} expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma omp declare

// expected-error@+3 {{function declaration is expected after 'declare variant' directive}}
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma options align=packed
int main();

// expected-error@+3 {{function declaration is expected after 'declare variant' directive}}
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma omp declare variant(foo) match(construct={target variant dispatch})
#pragma init_seg(compiler)
int main();

// expected-error@+1 {{single declaration is expected after 'declare variant' directive}}
#pragma omp declare variant(foo) match(construct={target variant dispatch})
int b, c;

void foo2(int*, int*, int*);
template <typename T>
void h(T *t1, T *t2, T *t3);

#pragma omp declare variant(foo2) match(construct={target variant dispatch})
template <>
void h(int *hp, int *hp2, int *hq) {
}

// expected-error@+1 {{variant function not found}}
#pragma omp declare variant(foo3) match(construct={target variant dispatch})
void bar();

int var1;
// expected-error@+1 {{variant must be a function}}
#pragma omp declare variant(var1) match(construct={target variant dispatch})
void bar();

// expected-error@+1 {{variant must be the name of a function or a template-id}}
#pragma omp declare variant(42) match(construct={target variant dispatch})
void bar();

// expected-error@+1 {{unknown or unsupported context selector set, expecting 'construct' or 'device'}}
#pragma omp declare variant(foo) match(onstruct={target variant dispatch})
void bar();

// expected-error@+1 {{unknown or unsupported 'construct' context selector, expecting 'target variant dispatch'}}
#pragma omp declare variant(foo) match(construct={arget})
void bar();

// expected-error@+1 {{unknown or unsupported 'device' context selector, expecting 'arch'}}
#pragma omp declare variant(foo) match(construct={target variant dispatch},device={rch(gen)})
void bar();

// expected-error@+1 {{unknown or unsupported 'arch' selector, expecting 'gen'}}
#pragma omp declare variant(foo) match(construct={target variant dispatch},device={arch(en)})
void bar();

void disp_call();

void testit() {
  int dnum = 1;
// expected-error@+1 {{cannot contain more than one 'device' clause}}
  #pragma omp target variant dispatch device(dnum) device(3)
  disp_call();
}

// expected-error@+3 {{unknown or unsupported 'construct' context selector, expecting 'target variant dispatch'}}
// expected-error@+3 {{unknown or unsupported 'arch' selector, expecting 'gen'}}
#pragma omp declare variant(foo) match(\
   construct={arget},\
   device={arch(gen),arch(foobar)})
void bar();

namespace N {
  // expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
  #pragma omp declare variant
}
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant

