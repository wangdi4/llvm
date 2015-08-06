// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility %s -isystem %S
// expected-no-diagnostics

#ifndef HEADER
#define HEADER
#include <cq373517_2.cpp>
#else
template <typename T>
T foo(T a = T());

template <typename T>
T foo(T a = T());
#endif // HEADER

