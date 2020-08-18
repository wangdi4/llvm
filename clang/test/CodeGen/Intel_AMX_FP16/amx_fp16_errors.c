// REQUIRES: intel_feature_isa_amx_fp16
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown  \
// RUN: -target-feature +amx-tile -target-feature +amx-int8 -target-feature +amx-fp16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// FP16
void test_tile_dpfp16ps() {
  _tile_dpfp16ps(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpfp16ps(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpfp16ps(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
}
