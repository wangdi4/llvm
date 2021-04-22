// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512f \
// RUN: -target-feature +avx512fp16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
void test_tile_tcvtrowps2pbf16hei() {
  _tile_tcvtrowps2pbf16hei(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2pbf16hei(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2pbf16hee(unsigned int A) {
  _tile_tcvtrowps2pbf16hee(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2phhei() {
  _tile_tcvtrowps2phhei(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2phhei(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2phhee(unsigned int A) {
  _tile_tcvtrowps2phhee(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2pbf16lei() {
  _tile_tcvtrowps2pbf16lei(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2pbf16lei(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2pbf16lee(unsigned int A) {
  _tile_tcvtrowps2pbf16lee(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2phlei() {
  _tile_tcvtrowps2phlei(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2phlei(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2phlee(unsigned int A) {
  _tile_tcvtrowps2phlee(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowd2psee(unsigned int A) {
  return _tile_tcvtrowd2psee(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowd2psei() {
  _tile_tcvtrowd2psei(32, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowd2psei(1, 256);  // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
