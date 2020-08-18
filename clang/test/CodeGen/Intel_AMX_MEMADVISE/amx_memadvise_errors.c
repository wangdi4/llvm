// REQUIRES: intel_feature_isa_amx_memadvise
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-reduce -target-feature +amx-memory \
// RUN: -target-feature +amx-format -target-feature +amx-element -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

void test_tile_movadvise(void *A) {
  _tile_movadvise_load(16, A, 8, 4); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_movadvise_store(A, 8, 16, 4); // expected-error {{argument value 16 is outside the valid range [0, 15]}}

}
