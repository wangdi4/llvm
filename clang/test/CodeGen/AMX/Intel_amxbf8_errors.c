// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown -target-feature +amx-tile -target-feature +amx-bf8 -emit-llvm -fsyntax-only -verify
// REQUIRES: intel_feature_isa_amx_bf8

#include <immintrin.h>

void test_amx(void *data) {
  _tile_dpbf8ps(4, 3, 3); // expected-error {{tile arguments must refer to different tiles}}
}
