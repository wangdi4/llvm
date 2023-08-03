// REQUIRES: intel_feature_isa_amx_element_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-element-evex \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
void test_tile_cvtd2pse(void *A, size_t B) {
  _tile_cvtd2pse(A, B, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
