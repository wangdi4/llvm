// REQUIRES: intel_feature_isa_amx_int8_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile -target-feature +amx-int8-evex    \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
// INT8-EVEX
void test_tile_tdpbssde() {
  _tile_dpbssde(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbssde(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbssde(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbssde(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbssde(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbssde(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbsude() {
  _tile_dpbsude(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbsude(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbsude(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbsude(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbsude(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbsude(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbusde() {
  _tile_dpbusde(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbusde(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbusde(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbusde(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbusde(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbusde(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbuude() {
  _tile_dpbuude(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbuude(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbuude(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_dpbuude(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbuude(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbuude(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
