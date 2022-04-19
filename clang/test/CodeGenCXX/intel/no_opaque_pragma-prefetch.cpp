// RUN: %clang_cc1 -emit-llvm -no-opaque-pointers -o - -fintel-compatibility \
// RUN: -triple x86_64-unknown-linux-gnu %s \
// RUN: | FileCheck %s
//
// Verify uses of #pragma prefetch
//
void func(int foo1[], int foo2[], int foo3[], int foo4[]) {
// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i8* null)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i8* null)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma noprefetch
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i8* null)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch *:3
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i8* null)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 10)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch *:3:10
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma noprefetch foo1, foo2
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1
  for (int i = 1; i < 100; i++) {
  }
//
// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch (foo1)
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1:2
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 3)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1:2:3
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1:2:3, foo2
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo3.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1:2:3, foo2, foo3:1
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo3.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 512)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo4.addr)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo1:1:3
#pragma prefetch foo2
#pragma prefetch foo3:0:512
#pragma noprefetch foo4
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo4.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo5)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo6)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 99)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo3.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 3)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 50)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
  float foo5[100];
  float foo6[100];
#pragma noprefetch foo4, foo5, foo6
#pragma prefetch foo1:1:99
#pragma prefetch foo2
#pragma prefetch foo3:3:50
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo6)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo5)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo4.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo3.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo1.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo6, foo5, foo4, foo3, foo2, foo1
  for (int i = 1; i < 100; i++) {
  }

// Verify mix of [no]prefetch with other loop directives.
// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo6)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo5)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo4.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo3.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo2.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma prefetch foo6
#pragma ivdep
#pragma prefetch foo5
#pragma loop_count(10)
#pragma prefetch foo4
#pragma nofusion
#pragma prefetch foo3
#pragma novector
#pragma noprefetch foo2
#pragma parallel
  for (int i = 1; i < 100; i++) {
  }

// Verify mix of [no]prefetch with other loop directives.
// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo6)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([100 x float]* %foo5)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** %foo4.addr)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma simd
#pragma prefetch foo6
#pragma unroll
#pragma noprefetch foo5
#pragma vector
#pragma prefetch foo4
  for (int i = 1; i < 100; i++) {
  }

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32* %j)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 10)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32* %k)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 99)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32* %arrayidx)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(%struct.foo* %bar3)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
  int index = 1;
  struct foo {
    int j;
    int k;
  } bar1, bar2, bar3;
#pragma prefetch bar1.j:1:10, bar2.k:2:99, foo1[index], bar3
  for (int i = 1; i < 100; i++) {
  }

  int p = 1;
  int *ptr = &p;
  int **pptr = &ptr;
// CHECK: [[LD1:%[0-9]*]] = load i32**, i32*** %pptr, align 8
// CHECK-NEXT: [[LD2:%[0-9]*]] = load i32**, i32*** %pptr, align 8
// CHECK-NEXT: [[LD3:%[0-9]*]] = load i32**, i32*** %pptr, align 8
//
// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 0)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** [[LD1]])
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** [[LD2]])
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32** [[LD3]])
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 2)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 10)
// CUECK: "DIR.PRAGMA.END.PREFETCH_LOOP"
#pragma noprefetch (*pptr)
#pragma prefetch (*pptr):1
#pragma prefetch (*pptr):2:10
  for (int i = 1; i < 100; i++) {
  }
}

class A {
public:
  static int i;
  static int j[10];
};

int A::i = 1;
int A::j[] = {0,1,2,3,4,5,6,7,8,9};

namespace B {
int i = 1;
int j[] = {0,1,2,3,4,5,6,7,8,9};

};

void prefetch_test(float *a, float *b, int *j, int n)
{
  int i;

// CHECK: DIR.PRAGMA.PREFETCH_LOOP
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32* @_Z{{[A-Z,a-z,0-9]+}})
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([10 x i32]* @_Z{{[A-Z,a-z,0-9]+}})
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 10)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"(i32* @_Z{{[A-Z,a-z,0-9]+}})
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.ENABLE"(i32 1)
// CHECK-SAME: "QUAL.PRAGMA.VAR"([10 x i32]* @_ZN1B1jE)
// CHECK-SAME: "QUAL.PRAGMA.HINT"(i32 -1)
// CHECK-SAME: "QUAL.PRAGMA.DISTANCE"(i32 -1)
// CHECK: DIR.PRAGMA.END.PREFETCH_LOOP
#pragma prefetch A::i
#pragma prefetch A::j:1:10
#pragma prefetch B::i
#pragma prefetch B::j
  for (i=0; i<n; i++)
  {
    a[i] = b[i];
    a[i] = a[i] + 1 + A::i + B::j[i];
    a[i] = a[i] + 2 + A::j[i] + B::i;
  }
}
