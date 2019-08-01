// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu \
// RUN:   -emit-pch -o cmplrllvm-220.h.gch \
// RUN:   -x c++-header %S/Inputs/cmplrllvm-220.h
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu \
// RUN:   -verify -emit-obj -o cmplrllvm-220.o \
// RUN:   -include-pch cmplrllvm-220.h.gch -x c++ %s

// expected-no-diagnostics

int C = A;

