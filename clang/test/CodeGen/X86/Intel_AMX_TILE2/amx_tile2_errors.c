// REQUIRES: intel_feature_isa_amx_tile2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-tile2 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
// AMX-tile2
void test_tile_tilemov()
{
  _tile_tilemov(8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_tilemov(1, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

