// REQUIRES: intel_feature_isa_amx_sparse
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-sparse -emit-llvm  \
// RUN: -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

void test_tile_dpsbssd_1() {
  _tile_dpsbssd(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbssd_2() {
  _tile_dpsbssd(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbssd_3() {
  _tile_dpsbssd(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbssd_4() {
  _tile_dpsbssd(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbssd_5() {
  _tile_dpsbssd(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbssd_6() {
  _tile_dpsbssd(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbsud_1() {
  _tile_dpsbsud(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbsud_2() {
  _tile_dpsbsud(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbsud_3() {
  _tile_dpsbsud(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbsud_4() {
  _tile_dpsbsud(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbsud_5() {
  _tile_dpsbsud(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbsud_6() {
  _tile_dpsbsud(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbusd_1() {
  _tile_dpsbusd(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbusd_2() {
  _tile_dpsbusd(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbusd_3() {
  _tile_dpsbusd(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbusd_4() {
  _tile_dpsbusd(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbusd_5() {
  _tile_dpsbusd(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbusd_6() {
  _tile_dpsbusd(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbuud_1() {
  _tile_dpsbuud(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbuud_2() {
  _tile_dpsbuud(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbuud_3() {
  _tile_dpsbuud(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbuud_4() {
  _tile_dpsbuud(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbuud_5() {
  _tile_dpsbuud(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbuud_6() {
  _tile_dpsbuud(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbf16ps_1() {
  _tile_dpsbf16ps(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbf16ps_2() {
  _tile_dpsbf16ps(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbf16ps_3() {
  _tile_dpsbf16ps(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsbf16ps_4() {
  _tile_dpsbf16ps(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbf16ps_5() {
  _tile_dpsbf16ps(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsbf16ps_6() {
  _tile_dpsbf16ps(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsfp16ps_1() {
  _tile_dpsfp16ps(0, 1, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsfp16ps_2() {
  _tile_dpsfp16ps(0, 2, 3); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsfp16ps_3() {
  _tile_dpsfp16ps(0, 2, 1); // expected-error {{tilepair arguments must refer to different tilepair registers}}
}

void test_tile_dpsfp16ps_4() {
  _tile_dpsfp16ps(0, 2, 8); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsfp16ps_5() {
  _tile_dpsfp16ps(0, 8, 2); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_dpsfp16ps_6() {
  _tile_dpsfp16ps(8, 2, 1); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_ldexpandb(const void *A, size_t B, unsigned int C) {
  _tile_ldexpandb(8, A, B, C); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_ldexpandbt1(const void *A, size_t B, unsigned int C) {
  _tile_ldexpandbt1(8, A, B, C); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_ldexpandw(const void *A, size_t B, unsigned int C) {
  _tile_ldexpandw(8, A, B, C); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}

void test_tile_ldexpandwt1(const void *A, size_t B, unsigned int C) {
  _tile_ldexpandwt1(8, A, B, C); // expected-error {{argument value 8 is outside the valid range [0, 7]}}
}
