// RUN: %clang_cc1 %s -fintel-compatibility-enable=UnderbarPragmaEnabled -verify=enabled
// RUN: %clang_cc1 %s -verify

// enabled-no-diagnostics
__pragma(warning(push))
// expected-error@-1 {{a type specifier is required for all declarations}}
// expected-error@-2 {{use of undeclared identifier 'push'}}
// expected-error@-3 {{expected ';' after top level declarator}}
