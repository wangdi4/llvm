// RUN: %clang_cc1 -verify -fintel-ms-compatibility \
// RUN:   -header-base-path "c:/foo/bar" %s
// RUN: %clang_cc1 -verify -fintel-ms-compatibility \
// RUN:   -header-base-path c:/foo/bar %s
// RUN: %clang_cc1 -verify -fintel-ms-compatibility \
// RUN:   -header-base-path %S %s
// REQUIRES: system-windows
//expected-no-diagnostics
