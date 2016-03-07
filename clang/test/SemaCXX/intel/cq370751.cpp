// RUN: %clang_cc1 %s -fsyntax-only -fintel-compatibility -verify

// intel-clang emits a warning, not an error, in case of 'inline'
// atttribute for non functions (standard clang emits an error ). CQ#370751.
__inline long a; // expected-warning {{'inline' can only appear on functions}} 

