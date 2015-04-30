// RUN: %clang_cc1 -IntelCompat -verify -emit-llvm %s  -o /dev/null

#pragma ident "foo"
#pragma ident "foo" "asasasasasa" asasa // expected-warning {{extra tokens at end of #pragma ident directive}}
