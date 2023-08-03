// REQUIRES: intel_feature_isa_amx_movrs
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-reduce -target-feature +amx-memory \
// RUN: -target-feature +amx-format -target-feature +amx-element -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

void test_tile_movadvise(void *A) {
  _tile_movadvise_load(8, A, 8, 4); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_movadvise_store(A, 8, 8, 4); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_t2rpntlvwz0advise(const void *A, size_t B) {
  _tile_t2rpntlvwz0advise(8, A, B, 127); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_t2rpntlvwz0advise(1, A, B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_t2rpntlvwz1advise(const void *A, size_t B) {
  _tile_t2rpntlvwz1advise(8, A, B, 127); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
  _tile_t2rpntlvwz0advise(1, A, B, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
