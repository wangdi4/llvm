// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify \
// RUN:  -triple spir64 %s
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify \
// RUN:  -triple spir %s
// RUN: %clang_cc1 -fsyntax-only \
// RUN:  -fintel-compatibility-enable=RelaxSpirCCNoProtoDiag -verify \
// RUN:  -triple spir %s
// RUN: %clang_cc1 -DERROR -fsyntax-only -verify -triple spir %s
// RUN: %clang_cc1 -DERROR -fsyntax-only -fintel-compatibility \
// RUN:  -fintel-compatibility-disable=RelaxSpirCCNoProtoDiag -verify \
// RUN:  -triple spir %s

#ifdef ERROR
void foo(); //  expected-error {{function with no prototype cannot use the spir_function calling convention}}
#else
void foo(); //  expected-warning {{function with no prototype cannot use the spir_function calling convention}}
#endif
