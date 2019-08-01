// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

struct S {} __attribute__((section(".data.cache"))); // expected-warning {{'section' attribute only applies to functions, global variables, Objective-C methods, and Objective-C properties}}

