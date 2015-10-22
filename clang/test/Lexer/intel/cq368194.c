// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s
// RUN: %clang_cc1 -fintel-compatibility -E %s | FileCheck %s

int x;
// expected-warning@+1{{unterminated /* comment}}
/*
double y;
// CHECK: int x
// CHECK-NOT: double y
