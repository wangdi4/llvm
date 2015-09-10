// CQ#370092
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

//
// Test verifies that we emit warnings (and not errors) on violations of
// attributes' usage.
//

typedef int i __attribute__((alias("j")));
// expected-warning@-1{{'alias' attribute only applies to functions and global variables}}

typedef int T __attribute__((__weakref__ ("U")));
// expected-warning@-1{{__weakref__' attribute only applies to variables and functions}}

struct __attribute((visibility("hidden"))) B;
struct __attribute((visibility("default"))) B;
// expected-warning@-1{{visibility does not match previous declaration; attribute ignored}}
// expected-note@-3{{previous attribute is here}}
