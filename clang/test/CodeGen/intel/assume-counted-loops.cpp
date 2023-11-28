// RUN: %clang_cc1 -fassume-counted-loops -triple x86_64-unknown-linux-gnu \
// RUN:   -verify -emit-llvm %s -o - | FileCheck %s

// RUN: %clang_cc1 -fno-assume-counted-loops -triple x86_64-unknown-linux-gnu \
// RUN:   -verify -emit-llvm %s -o - | FileCheck %s

// expected-no-diagnostics

struct S {
  int Size;
  int Increment;
  int *Elem1;
  int *Elem2;
};

// CHECK: define{{.*}}test1
void test1(S *P, int N) {
  for (unsigned int I = 0; I < P->Size; I += P->Increment) {
    P->Elem1[I] = -1;
    P->Elem2[I] = 0;
  }
}
