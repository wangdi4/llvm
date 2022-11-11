// REQUIRES: intel_feature_isa_amx_avx512_cvtrow
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-avx512-cvtrow -target-feature +avx512f \
// RUN: -target-feature +avx512fp16 -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>
void test_tile_cvtrowps2pbf16hi() {
  _tile_cvtrowps2pbf16hi(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvtrowps2pbf16hi(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_cvtrowps2pbf16h(unsigned int A) {
  _tile_cvtrowps2pbf16h(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtrowps2phhi() {
  _tile_cvtrowps2phhi(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvtrowps2phhi(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_cvtrowps2phh(unsigned int A) {
  _tile_cvtrowps2phh(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtrowps2pbf16li() {
  _tile_cvtrowps2pbf16li(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvtrowps2pbf16li(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_cvtrowps2pbf16l(unsigned int A) {
  _tile_cvtrowps2pbf16l(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtrowps2phli() {
  _tile_cvtrowps2phli(32, 1);   // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvtrowps2phli(31, 256); // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}

void test_tile_cvtrowps2phl(unsigned int A) {
  _tile_cvtrowps2phl(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtrowd2ps(unsigned int A) {
  return _tile_cvtrowd2ps(32, A); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_cvtrowd2psi() {
  _tile_cvtrowd2psi(32, 127); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_cvtrowd2psi(1, 256);  // expected-error {{argument value 256 is outside the valid range [0, 255]}}
}
