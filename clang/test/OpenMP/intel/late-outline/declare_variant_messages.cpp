// INTEL_COLLAB
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

// expected-error@+1 {{use of undeclared identifier 'foo3'}}
#pragma omp declare variant(foo3) match(construct={target variant dispatch})
void bar();

int var1;
// expected-error@+1 {{variant in '#pragma omp declare variant' with type 'int' is incompatible with type 'void ()'}}
#pragma omp declare variant(var1) match(construct={target variant dispatch})
void bar();

// expected-error@+1 {{variant in '#pragma omp declare variant' with type 'int' is incompatible with type 'void ()'}}
#pragma omp declare variant(42) match(construct={target variant dispatch})
void bar();

// expected-warning@+3 {{'onstruct' is not a valid context set in a `declare variant`}}
// expected-note@+2 {{context set options are}}
// expected-note@+1 {{the ignored set spans until here}}
#pragma omp declare variant(foo) match(onstruct={target variant dispatch})
void bar(int);

// expected-warning@+3 {{'arget' is not a valid context selector}}
// expected-note@+2 {{context selector options are}}
// expected-note@+1 {{the ignored selector spans until here}}
#pragma omp declare variant(foo) match(construct={arget})
void bar(int);

// expected-warning@+3 {{'rch' is not a valid context selector for the context set 'device'}}
// expected-note@+2 {{context selector options are}}
// expected-note@+1 {{the ignored selector spans until here}}
#pragma omp declare variant(foo) match(construct={target variant dispatch},device={rch(gen)})
void bar(int);

// expected-warning@+3 {{'en' is not a valid context property for the context selector 'arch' and the context set 'device'}}
// expected-note@+2 {{context property options are}}
// expected-note@+1 {{the ignored property spans until here}}
#pragma omp declare variant(foo) match(construct={target variant dispatch},device={arch(en)})
void bar(int);

void disp_call();

void testit() {
  int dnum = 1;
// expected-error@+1 {{cannot contain more than one 'device' clause}}
  #pragma omp target variant dispatch device(dnum) device(3)
  disp_call();
// expected-error@+1 {{cannot contain more than one 'nowait' clause}}
  #pragma omp target variant dispatch nowait device(dnum) nowait
  disp_call();
}

// expected-warning@+7 {{'arget' is not a valid context selector for the context set 'construct'}}
// expected-note@+6 {{context selector options are}}
// expected-note@+5 {{the ignored selector spans}}
// expected-warning@+5 {{'foobar' is not a valid context property for the context selector 'arch' and the context set 'device'}}
// expected-note@+4 {{context property options are}}
// expected-note@+3 {{the ignored property spans}}
#pragma omp declare variant(foo) match(\
   construct={arget},\
   device={arch(gen,foobar)})
void bar(int);

namespace N {
  // expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
  #pragma omp declare variant
}
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant
// expected-error@+1 {{function declaration is expected after 'declare variant' directive}}
#pragma omp declare variant

// end INTEL_COLLAB
