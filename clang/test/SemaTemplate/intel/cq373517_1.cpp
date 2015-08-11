// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -fpermissive-args %s
// expected-no-diagnostics

template <typename T>
T foo(T a = T());

template <typename T>
T foo(T a = T());


