// RUN: %clang_cc1 -fintel-compatibility -E %s | grep '#pragma foo bar'
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -Wunknown-pragmas -verify %s

// GCC doesn't expand macro args for unrecognized pragmas.
#define bar xX
#pragma foo bar   // expected-warning {{unknown pragma ignored}}

#pragma fp_contract(on)
#pragma fp_contract (off)
#pragma fp_contract (default) // expected-warning {{'on' or 'off' is expected}}
#pragma fp_contract (IN_BETWEEN)  // expected-warning {{'on' or 'off' is expected}}

#pragma fenv_access (on) // expected-error {{fenv_access cannot be enabled except in precise, source, double and extended modes}}
#pragma fenv_access (off)
#pragma fenv_access (default)       // expected-warning {{'on' or 'off' is expected}}
#pragma fenv_access (IN_BETWEEN)   // expected-warning {{'on' or 'off' is expected}}

#pragma fenv_access    // expected-warning {{missing '(' after '#pragma fenv_access' - ignoring}}
