// RUN: %clang_cc1 -verify -fintel-compatibility %s
//
// Test parsing warnings/errors related to #pragma [no]prefetch
int *foobar() {
  int *i = new int(1);

  return i;
}

void prefetch() {
  int A;
  int B;
  int p = 1;
  int pa = 2;
  int pb = 3;
  int pc = 4;
  int *pp = &p;
  struct S {
    int i;
    int f() {
      return i;
    }
  };

  S s = {1};
// expected-error@+1 {{invalid expression in '#pragma noprefetch'}}
#pragma noprefetch *
// expected-error@+1 {{invalid expression in '#pragma noprefetch'}}
#pragma noprefetch *:
// expected-error@+1 {{invalid expression in '#pragma noprefetch'}}
#pragma noprefetch 1:2
// expected-error@+1 {{expected ':'}}
#pragma prefetch *
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch *:
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch *:1:
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch (10:15)
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch A:A
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch A:1:
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch'}}
#pragma prefetch *:1, A
// expected-error@+1 {{expected ':'}}
#pragma prefetch *(pp+1)
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch s.f()
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch foobar()
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch p + 1
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch' - ignored}}
#pragma prefetch *:1:2:3
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch' - ignored}}
#pragma prefetch p:1:2:3
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch p:1:pa, pa:2, pb, pc:1
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch 6:41:2
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch p:2:4, 6
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch p:2:4, *:1
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch' - ignored}}
#pragma prefetch *:2:4, *:1
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch (1+2):2:3
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch' - ignored}}
#pragma prefetch p:2:3 4
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch 10:15
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch p:1,1
// expected-error@+1 {{expected ':'}}
#pragma prefetch *, *:2
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch *:, *:2
// expected-warning@+1 {{extra tokens at end of '#pragma prefetch' - ignored}}
#pragma prefetch *:1, *:2
// expected-error@+1 {{expected an integer constant in '#pragma prefetch'}}
#pragma prefetch *:1:, *:2
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch (10:15)
// expected-error@+1 {{invalid expression in '#pragma prefetch'}}
#pragma prefetch (*):1
  for(int i=0; i<10; ++i) {}
}
