// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512f \
// RUN: -target-feature +avx512fp16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
void test_tile_tcvtrowps2pbf16hi() {
  _tile_tcvtrowps2pbf16hi(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2pbf16hi(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2pbf16he(unsigned int A) {
  _tile_tcvtrowps2pbf16he(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2phhi() {
  _tile_tcvtrowps2phhi(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2phhi(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2phhe(unsigned int A) {
  _tile_tcvtrowps2phhe(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2pbf16li() {
  _tile_tcvtrowps2pbf16li(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2pbf16li(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2pbf16le(unsigned int A) {
  _tile_tcvtrowps2pbf16le(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowps2phli() {
  _tile_tcvtrowps2phli(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowps2phli(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_tcvtrowps2phle(unsigned int A) {
  _tile_tcvtrowps2phle(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowd2pse(unsigned int A) {
  return _tile_tcvtrowd2pse(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tcvtrowd2psi() {
  _tile_tcvtrowd2psi(32, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tcvtrowd2psi(1, 256);  // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
