// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux -fsyntax-only %s -ast-dump -verify | FileCheck %s
// REQUIRES: non-ms-sdk

double D1 = 2e40f; // expected-warning {{magnitude of floating-point constant too large for type 'float'}}
// CHECK: -FloatingLiteral {{.*}} 'float' INF
double D2 = 2e40d; // expected-warning {{d-suffix in FP literal}}
// CHECK: -FloatingLiteral {{.*}} 'double' 2.000000e+40
double D3 = 2e400d; // expected-warning {{d-suffix in FP literal}}
// expected-warning@-1 {{magnitude of floating-point constant too large for type 'double'}}
// CHECK: -FloatingLiteral {{.*}} 'double' INF

