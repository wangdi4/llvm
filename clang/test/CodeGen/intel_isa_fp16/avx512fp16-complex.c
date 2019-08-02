// REQUIRES: intel_feature_isa_fp16
// RUN: %clang_cc1 -verify -triple=x86_64-unknown-unknown -target-feature +avx512fp16 %s

// We don't support complex of _Float16 due to missing runtime library support
// for some arithmetic operations.

_Complex _Float16 cmplx_fp16;  // expected-error{{'_Complex _Float16' is invalid}}
