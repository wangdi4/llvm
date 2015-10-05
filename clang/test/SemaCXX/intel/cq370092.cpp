// CQ#370092
// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -std=c++0x -verify %s

//
// Test verifies that we emit warnings (and not errors) on violations of
// attributes' usage. This is C++-specific version of the test.
//

typedef int [[gnu::warn_unused_result]] (*F5) (int);
// expected-warning@-1{{'warn_unused_result' attribute cannot be applied to types}}

static [[gnu::noreturn]] void two (void) {}
// expected-warning@-1{{an attribute list cannot appear here; attributes are ignored}}

int foo ()
{
good_ignored : [[gnu::unused]]
// expected-warning@-1{{'unused' attribute cannot be applied to a statement; attribute ignored}}
 return 0;
}

