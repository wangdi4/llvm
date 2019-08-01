// RUN: %clang_cc1 -ast-dump -verify %s | FileCheck %s -check-prefix=NOHLS
// RUN: %clang_cc1 -ast-dump -verify -fhls -DHLS %s | FileCheck %s -check-prefix=HLS

#if defined(HLS)
// expected-no-diagnostics
#else
// expected-warning@+2{{'__ap_int' attribute ignored}}
#endif
typedef int ap_int __attribute__((__ap_int(5)));
// HLS: ArbPrecIntType
// NOHLS-NOT: ArbPrecIntType
