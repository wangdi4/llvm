// REQUIRES: intel_feature_isa_amx2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-int8-evex -target-feature +amx-bf16-evex -target-feature +amx-tile-evex -target-feature\
// RUN: +amx-transpose -target-feature +amx-reduce -target-feature +amx-memory -target-feature +amx-format -target-feature +amx-element -target-feature +amx-fp16 \
// RUN: -target-feature +amx-avx512 -target-feature +avx512f \
// RUN: -emit-llvm -fsyntax-only -verify

#include <immintrin.h>
#include <stddef.h>

// Transpose
void test_tile_2rpntlvw(const void *A, size_t B, unsigned short C) {
  _tile_2rpntlvw(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}
void test_tile_2rpntlvwt1(const void *A, size_t B, unsigned short C) {
  _tile_2rpntlvwt1(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}
void test_tile_2transposew(const void *A, size_t B, unsigned short C) {
  _tile_2transposew(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}
void test_tile_2transposewt1(const void *A, size_t B, unsigned short C) {
  _tile_2transposewt1(16, A, B, C); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

// Reduce
void test_tile_coladdbcastps() {
  _tile_coladdbcastps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_coladdps(void *A) {
  _tile_coladdps(A, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

// Memory
void test_tile_broadcastrowd(const void *A) {
  _tile_broadcastrowd(16, A); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_gatherrowd(const void *A, const void *B) {
  _tile_gatherrowd(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}

}

void test_tile_gatherrowdt1(const void *A, const void *B) {
  _tile_gatherrowdt1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_gatherrowq(const void *A, const void *B) {
  _tile_gatherrowq(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_gatherrowqt1(const void *A, const void *B) {
  _tile_gatherrowqt1(16, A, B); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_scatterrowd(void *A, void *B) {
  _tile_scatterrowd(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_scatterrowdt1(void *A, void *B) {
  _tile_scatterrowdt1(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_scatterrowq(void *A, void *B) {
  _tile_scatterrowq(A, B, 16);  // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_scatterrowqt1(void *A, void *B) {
  _tile_scatterrowqt1(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storehd(void *A, size_t B) {
  _tile_storehd(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storehdt1(void *A, size_t B) {
  _tile_storehdt1(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storentd(void *A, size_t B) {
  _tile_storentd(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storeqd(void *A, size_t B) {
  _tile_storeqd(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storeqdt1(void *A, size_t B) {
  _tile_storeqdt1(A, B, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

void test_tile_storerowd(void *A) {
  _tile_storerowd(A, 16); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

// Format
void test_tile_blendvd() {
  _tile_blendvd(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_blendvd(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_blendvd(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_interleaveeb() {
  _tile_interleaveeb(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveeb(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveeb(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_interleaveew() {
  _tile_interleaveew(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveew(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveew(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_interleaveob() {
  _tile_interleaveob(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveob(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveob(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_interleaveow() {
  _tile_interleaveow(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveow(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_interleaveow(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_narrowb() {
  _tile_narrowb(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_narroww() {
  _tile_narroww(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permb_reg() {
  _tile_permb_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permb_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permb_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permb_mem(const void *A) {
  _tile_permb_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permd_reg() {
  _tile_permd_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permd_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permd_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permd_mem(const void *A) {
  _tile_permd_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permw_reg() {
  _tile_permw_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permw_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_permw_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_permw_mem(const void *A) {
  _tile_permw_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_widenb() {
  _tile_widenb(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_widenw() {
  _tile_widenw(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

// Element
void test_tile_addps_reg() {
  _tile_addps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_addps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_addps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_addps_mem(const void *A) {
  _tile_addps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_andd_reg() {
  _tile_andd_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_andd_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_andd_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_andd_mem(const void *A) {
  _tile_andd_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_andnd_reg() {
  _tile_andnd_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_andnd_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_andnd_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_andnd_mem(const void *A) {
  _tile_andnd_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cmpps_reg() {
  _tile_cmpps_reg(1, 1, 3, 127); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_cmpps_reg(1, 2, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_cmpps_reg(1, 2, 2, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cmpps_mem(const void *A) {
  _tile_cmpps_mem(1, 1, A, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtb2ps() {
  _tile_cvtb2ps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtbf162ps() {
  _tile_cvtbf162ps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtd2ps() {
  _tile_cvtd2ps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtps2bf16() {
  _tile_cvtps2bf16(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtps2bs() {
  _tile_cvtps2bs(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtps2ubs() {
  _tile_cvtps2ubs(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_cvtub2ps() {
  _tile_cvtub2ps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fmaddps_reg() {
  _tile_fmaddps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fmaddps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fmaddps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fmaddps_mem(const void *A) {
  _tile_fmaddps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fmsubps_reg() {
  _tile_fmsubps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fmsubps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fmsubps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fmsubps_mem(const void *A) {
  _tile_fmsubps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fnmaddps_reg() {
  _tile_fnmaddps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fnmaddps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fnmaddps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fnmaddps_mem(const void *A) {
  _tile_fnmaddps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fnmsubps_reg() {
  _tile_fnmsubps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fnmsubps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_fnmsubps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_fnmsubps_mem(const void *A) {
  _tile_fnmsubps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_maxps_reg() {
  _tile_maxps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_maxps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_maxps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_maxps_mem(const void *A) {
  _tile_maxps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_minps_reg() {
  _tile_minps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_minps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_minps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_minps_mem(const void *A) {
  _tile_minps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_mulps_reg() {
  _tile_mulps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_mulps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_mulps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_mulps_mem(const void *A) {
  _tile_mulps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_ord_reg() {
  _tile_ord_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_ord_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_ord_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_ord_mem(const void *A) {
  _tile_ord_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_rcp14ps() {
  _tile_rcp14ps(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_reduceps() {
  _tile_reduceps(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_scalefps_reg() {
  _tile_scalefps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_scalefps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_scalefps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_scalefps_mem(const void *A) {
  _tile_scalefps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_slld() {
  _tile_slld(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_srld() {
  _tile_srld(1, 1, 127); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_srlvd_reg() {
  _tile_srlvd_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_srlvd_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_srlvd_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_srlvd_mem(void *A) {
  _tile_srlvd_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_subps_reg() {
  _tile_subps_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_subps_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_subps_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_subps_mem(void *A) {
  _tile_subps_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_xord_reg() {
  _tile_xord_reg(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_xord_reg(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_xord_reg(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

void test_tile_xord_mem(void *A) {
  _tile_xord_mem(1, 1, A); // expected-error {{tmul arguments must refer to different tiles}}
}

// FP16
void test_tile_dpfp16ps() {
  _tile_dpfp16ps(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpfp16ps(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpfp16ps(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
}

// AMXAVX512
typedef float __m512 __attribute__((__vector_size__(64)));
void test_tile_mov16zmm(__m512 tsrc1, __m512 tsrc2, __m512 tsrc3, __m512 tsrc4,
  __m512 tsrc5, __m512 tsrc6, __m512 tsrc7, __m512 tsrc8, __m512 tsrc9,
  __m512 tsrc10, __m512 tsrc11, __m512 tsrc12, __m512 tsrc13, __m512 tsrc14,
  __m512 tsrc15, __m512 tsrc16) {
  _tile_tile16move(16, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7, tsrc8,// expected-error {{argument value 16 is outside the valid range [0, 15]}}
              tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16);
}

void test_tile_tilemovrowei() {
  _tile_tilemovrowei(16, 1); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

typedef unsigned int uint32_t;
void test_tile_tilemovrowee(uint32_t A) {
  _tile_tilemovrowee(16, A); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

typedef float __m128 __attribute__((__vector_size__(16)));
void test_tile_tilemovrowex(__m128 A) {
  _tile_tilemovrowex(16, A); // expected-error {{argument value 16 is outside the valid range [0, 15]}}
}

// BF16-EVEX
void test_tile_dpbf16pse() {
  _tile_dpbf16pse(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbf16pse(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbf16pse(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbf16pse(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbf16pse(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbf16pse(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

// INT8-EVEX
void test_tile_tdpbssde() {
  _tile_dpbssde(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbssde(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbssde(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbssde(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbssde(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbssde(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbsude() {
  _tile_dpbsude(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbsude(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbsude(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbsude(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbsude(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbsude(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbusde() {
  _tile_dpbusde(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbusde(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbusde(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbusde(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbusde(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbusde(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tdpbuude() {
  _tile_dpbuude(1, 1, 3); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbuude(1, 2, 2); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbuude(1, 2, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_dpbuude(32, 2, 1); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbuude(1, 32, 3); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_dpbuude(1, 2, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

// TILE-EVEX
void test_tile_loadde(const void * base, size_t stride) {
  _tile_loadde(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tileloaddt164e(const void * base, size_t stride) {
  _tile_stream_loadde(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilestored64e(const void * base, size_t stride) {
  _tile_storede(32, base, stride); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilemove() {
  _tile_tilemove(1, 1); // expected-error {{tmul arguments must refer to different tiles}}
  _tile_tilemove(32, 2); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
  _tile_tilemove(1, 32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}

void test_tile_tilezeroe() {
  _tile_zeroe(32); // expected-error {{argument value 32 is outside the valid range [0, 31]}}
}
