// RUN: %clang_cc1 -fintel-compatibility -E %s

#ident "string"
#ident nostring // expected-warning{{invalid #ident directive will be ignored}}
