// REQUIRES: intel_feature_isa_amx_transpose2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-fp16 -target-feature +amx-transpose2 \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// Transpose2
void test_tile_2transposew(const void *A, size_t B, unsigned short C) {
  _tile_2transposew(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_2transposewt1(const void *A, size_t B, unsigned short C) {
  _tile_2transposewt1(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz0(const void *A, size_t B) {
  _tile_4rqntlvbz0(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz0t1(const void *A, size_t B) {
  _tile_4rqntlvbz0t1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz1(const void *A, size_t B) {
  _tile_4rqntlvbz1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz1t1(const void *A, size_t B) {
  _tile_4rqntlvbz1t1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz2(const void *A, size_t B) {
  _tile_4rqntlvbz2(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz2t1(const void *A, size_t B) {
  _tile_4rqntlvbz2t1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz3(const void *A, size_t B) {
  _tile_4rqntlvbz3(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_4rqntlvbz3t1(const void *A, size_t B) {
  _tile_4rqntlvbz3t1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_tdpbssd() {
  _tile_tdpbssd(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbssd(1, 2, 1);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbssd(1, 2, 2);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbssd(16, 2, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbssd(1, 16, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbssd(1, 2, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_tdpbsud() {
  _tile_tdpbsud(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbsud(1, 2, 1);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbsud(1, 2, 2);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbsud(16, 2, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbsud(1, 16, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbsud(1, 2, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_tdpbusd() {
  _tile_tdpbusd(1, 1, 3);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbusd(1, 2, 1);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbusd(1, 2, 2);  // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbusd(16, 2, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbusd(1, 16, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbusd(1, 2, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}

}

void test_tile_tdpbuud() {
  _tile_tdpbuud(1, 1, 3); // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbuud(1, 2, 1); // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbuud(1, 2, 2); // expected-error {{tile arguments must refer to different tiles}}
  _tile_tdpbuud(16, 2, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbuud(1, 16, 2); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
  _tile_tdpbuud(1, 2, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

