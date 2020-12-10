// REQUIRES: intel_feature_isa_amx_memadvise_evex
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-memadvise-evex -emit-llvm -fsyntax-only -verify
//
#include <immintrin.h>
#include <stddef.h>

void test_tile_t2rpntlvwz0advisee(const void *A, size_t B) {
  _tile_t2rpntlvwz0advisee(32, A, B, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_t2rpntlvwz0advisee(1, A, B, 256);  // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_t2rpntlvwz1advisee(const void *A, size_t B) {
  _tile_t2rpntlvwz1advisee(32, A, B, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_t2rpntlvwz1advisee(1, A, B, 256);  // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
