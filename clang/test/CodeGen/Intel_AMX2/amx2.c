// REQUIRES: intel_feature_isa_amx2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-int8-evex -target-feature +amx-bf16-evex -target-feature +amx-tile-evex -target-feature\
// RUN: +avx512f -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
// Transpose
void test_tile_2rpntlvw(int A, void *B, int C) {
  // CHECK-LABEL: @test_tile_2rpntlvw
  // CHECK: call void asm sideeffect "t2rpntlvw $0, ($1,$2,8), %tmm1"
  _tile_2rpntlvw(A, B, C, 8, 1);
}

void test_tile_2rpntlvwt1(int A, void *B, int C) {
  // CHECK-LABEL: @test_tile_2rpntlvwt1
  // CHECK: call void asm sideeffect "t2rpntlvwt1 $0, ($1,$2,8), %tmm1"
  _tile_2rpntlvwt1(A, B, C, 8, 1);
}

void test_tile_2transposew(int A, void *B, int C) {
  // CHECK-LABEL: @test_tile_2transposew
  // CHECK: call void asm sideeffect "t2transposew $0, ($1,$2,8), %tmm1"
  _tile_2transposew(A, B, C, 8, 1);
}

void test_tile_2transposewt1(int A, void *B, int C) {
  // CHECK-LABEL: @test_tile_2transposewt1
  // CHECK: call void asm sideeffect "t2transposewt1 $0, ($1,$2,8), %tmm1"
  _tile_2transposewt1(A, B, C, 8, 1);
}
// Reduce
void test_tile_coladdbcastps() {
  // CHECK-LABEL: @test_tile_coladdbcastps
  // CHECK: call void asm sideeffect "tcoladdbcastps %tmm1, %tmm2"
  _tile_coladdbcastps(1, 2);
}

void test_tile_coladdps(void *A) {
  // CHECK-LABEL: @test_tile_coladdps
  // CHECK: call void asm sideeffect "tcoladdps %tmm1, $0"
  _tile_coladdps(1, A);
}
// Memory
void test_tile_broadcastrowd(void *A) {
  // CHECK-LABEL: @test_tile_broadcastrowd
  // CHECK: call void asm sideeffect "tbroadcastrowd $0, %tmm1"
  _tile_broadcastrowd(A, 1);
}

void test_tile_gatherrowd(void *A, void *B) {
  // CHECK-LABEL: @test_tile_gatherrowd
  // CHECK: call void asm sideeffect "tgatherrowd ($0,$1), %tmm1"
  _tile_gatherrowd(A, B, 1);
}

void test_tile_gatherrowdt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_gatherrowdt1
  // CHECK: call void asm sideeffect "tgatherrowdt1 ($0,$1), %tmm1"
  _tile_gatherrowdt1(A, B, 1);
}

void test_tile_gatherrowq(void *A, void *B) {
  // CHECK-LABEL: @test_tile_gatherrowq
  // CHECK: call void asm sideeffect "tgatherrowq ($0,$1), %tmm1"
  _tile_gatherrowq(A, B, 1);
}

void test_tile_gatherrowqt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_gatherrowqt1
  // CHECK: call void asm sideeffect "tgatherrowqt1 ($0,$1), %tmm1"
  _tile_gatherrowqt1(A, B, 1);
}

void test_tile_scatterrowd(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowd
  // CHECK: call void asm sideeffect "tscatterrowd %tmm1, ($0,$1)"
  _tile_scatterrowd(1, A, B);
}

void test_tile_scatterrowdt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowdt1
  // CHECK: call void asm sideeffect "tscatterrowdt1 %tmm1, ($0,$1)"
  _tile_scatterrowdt1(1, A, B);
}

void test_tile_scatterrowq(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowq
  // CHECK: call void asm sideeffect "tscatterrowq %tmm1, ($0,$1)"
  _tile_scatterrowq(1, A, B);
}

void test_tile_scatterrowqt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowqt1
  // CHECK: call void asm sideeffect "tscatterrowqt1 %tmm1, ($0,$1)"
  _tile_scatterrowqt1(1, A, B);
}

void test_tile_storehd(void *A, int B) {
  // CHECK-LABEL: @test_tile_storehd
  // CHECK: call void asm sideeffect "tstorehd %tmm1, ($0,$1,8)"
  _tile_storehd(1, A, B, 8);
}

void test_tile_storehdt1(void *A, int B) {
  // CHECK-LABEL: @test_tile_storehdt1
  // CHECK: call void asm sideeffect "tstorehdt1 %tmm1, ($0,$1,8)"
  _tile_storehdt1(1, A, B, 8);
}

void test_tile_storentd(void *A, int B) {
  // CHECK-LABEL: @test_tile_storentd
  // CHECK: call void asm sideeffect "tstorentd %tmm1, ($0,$1,8)"
  _tile_storentd(1, A, B, 8);
}

void test_tile_storeqd(void *A, int B) {
  // CHECK-LABEL: @test_tile_storeqd
  // CHECK: call void asm sideeffect "tstoreqd %tmm1, ($0,$1,8)"
  _tile_storeqd(1, A, B, 8);
}

void test_tile_storeqdt1(void *A, int B) {
  // CHECK-LABEL: @test_tile_storeqdt1
  // CHECK: call void asm sideeffect "tstoreqdt1 %tmm1, ($0,$1,8)"
  _tile_storeqdt1(1, A, B, 8);
}

void test_tile_storerowd(void *A) {
  // CHECK-LABEL: @test_tile_storerowd
  // CHECK: call void asm sideeffect "tstorerowd %tmm1, $0"
  _tile_storerowd(1, A);
}
// Format
void test_tile_blendvd() {
  // CHECK-LABEL: @test_tile_blendvd
  // CHECK: call void asm sideeffect "tblendvd %tmm1, %tmm2, %tmm3"
  _tile_blendvd(1, 2, 3);
}

void test_tile_interleaveeb() {
  // CHECK-LABEL: @test_tile_interleaveeb
  // CHECK: call void asm sideeffect "tinterleaveeb %tmm1, %tmm2, %tmm3"
  _tile_interleaveeb(1, 2, 3);
}

void test_tile_interleaveew() {
  // CHECK-LABEL: @test_tile_interleaveew
  // CHECK: call void asm sideeffect "tinterleaveew %tmm1, %tmm2, %tmm3"
  _tile_interleaveew(1, 2, 3);
}

void test_tile_interleaveob() {
  // CHECK-LABEL: @test_tile_interleaveob
  // CHECK: call void asm sideeffect "tinterleaveob %tmm1, %tmm2, %tmm3"
  _tile_interleaveob(1, 2, 3);
}

void test_tile_interleaveow() {
  // CHECK-LABEL: @test_tile_interleaveow
  // CHECK: call void asm sideeffect "tinterleaveow %tmm1, %tmm2, %tmm3"
  _tile_interleaveow(1, 2, 3);
}

void test_tile_narrowb() {
  // CHECK-LABEL: @test_tile_narrowb
  // CHECK: call void asm sideeffect "tnarrowb $0, %tmm1, %tmm2",
  _tile_narrowb(128, 1, 2);
}

void test_tile_narroww() {
  // CHECK-LABEL: @test_tile_narroww
  // CHECK: call void asm sideeffect "tnarroww $0, %tmm1, %tmm2"
  _tile_narroww(128, 1, 2);
}

void test_tile_permb_reg() {
  // CHECK-LABEL: @test_tile_permb_reg
  // CHECK: call void asm sideeffect "tpermb %tmm1, %tmm2, %tmm3"
  _tile_permb_reg(1, 2, 3);
}

void test_tile_permb_mem(void *A) {
  // CHECK-LABEL: @test_tile_permb_mem
  // CHECK: call void asm sideeffect "tpermb $0, %tmm2, %tmm3"
  _tile_permb_mem(A, 2, 3);
}

void test_tile_permd_reg() {
  // CHECK-LABEL: @test_tile_permd_reg
  // CHECK: call void asm sideeffect "tpermd %tmm1, %tmm2, %tmm3"
  _tile_permd_reg(1, 2, 3);
}

void test_tile_permd_mem(void *A) {
  //  CHECK-LABEL: @test_tile_permd_mem
  //  CHECK: call void asm sideeffect "tpermd $0, %tmm2, %tmm3"
  _tile_permd_mem(A, 2, 3);
}

void test_tile_permw_reg() {
  // CHECK-LABEL: @test_tile_permw_reg
  // CHECK: call void asm sideeffect "tpermw %tmm1, %tmm2, %tmm3"
  _tile_permw_reg(1, 2, 3);
}

void test_tile_permw_mem(void *A) {
  // CHECK-LABEL: @test_tile_permw_mem
  // CHECK: call void asm sideeffect "tpermw $0, %tmm2, %tmm3"
  _tile_permw_mem(A, 2, 3);
}

void test_tile_widenb() {
  // CHECK-LABEL: @test_tile_widenb
  // CHECK: call void asm sideeffect "twidenb $0, %tmm1, %tmm2"
  _tile_widenb(128, 1, 2);
}

void test_tile_widenw() {
  // CHECK-LABEL: @test_tile_widenw
  // CHECK: call void asm sideeffect "twidenw $0, %tmm1, %tmm2"
  _tile_widenw(128, 1, 2);
}
// Element
void test_tile_addps_reg() {
  // CHECK-LABEL: @test_tile_addps_reg
  // CHECK: call void asm sideeffect "taddps %tmm1, %tmm2, %tmm3"
  _tile_addps_reg(1, 2, 3);
}

void test_tile_addps_mem(void *A) {
  // CHECK-LABEL: @test_tile_addps_mem
  // CHECK: call void asm sideeffect "taddps $0, %tmm2, %tmm3"
  _tile_addps_mem(A, 2, 3);
}

void test_tile_andd_reg() {
  // CHECK-LABEL: @test_tile_andd_reg
  // CHECK: call void asm sideeffect "tandd %tmm1, %tmm2, %tmm3"
  _tile_andd_reg(1, 2, 3);
}

void test_tile_andd_mem(void *A) {
  // CHECK-LABEL: @test_tile_andd_mem
  // CHECK: call void asm sideeffect "tandd $0, %tmm2, %tmm3"
  _tile_andd_mem(A, 2, 3);
}

void test_tile_andnd_reg() {
  // CHECK-LABEL: @test_tile_andnd_reg
  // CHECK: call void asm sideeffect "tandnd %tmm1, %tmm2, %tmm3
  _tile_andnd_reg(1, 2, 3);
}

void test_tile_andnd_mem(void *A) {
  // CHECK-LABEL: @test_tile_andnd_mem
  // CHECK: call void asm sideeffect "tandnd $0, %tmm2, %tmm3"
  _tile_andnd_mem(A, 2, 3);
}

void test_tile_cmpps_reg() {
  // CHECK-LABEL: @test_tile_cmpps_reg
  // CHECK: call void asm sideeffect "tcmpps $0, %tmm1, %tmm2, %tmm3"
  _tile_cmpps_reg(128, 1, 2, 3);
}

void test_tile_cmpps_mem(void *A) {
  // CHECK-LABEL: @test_tile_cmpps_mem
  // CHECK: call void asm sideeffect "tcmpps $0, $1, %tmm2, %tmm3"
  _tile_cmpps_mem(128, A, 2, 3);
}

void test_tile_cvtb2ps() {
  // CHECK-LABEL: @test_tile_cvtb2ps
  // CHECK: call void asm sideeffect "tcvtb2ps %tmm1, %tmm2"
  _tile_cvtb2ps(1, 2);
}

void test_tile_cvtbf162ps() {
  // CHECK-LABEL: @test_tile_cvtbf162ps
  // CHECK: call void asm sideeffect "tcvtbf162ps %tmm1, %tmm2"
  _tile_cvtbf162ps(1, 2);
}

void test_tile_cvtd2ps() {
  // CHECK-LABEL: @test_tile_cvtd2ps
  // CHECK: call void asm sideeffect "tcvtd2ps %tmm1, %tmm2"
  _tile_cvtd2ps(1, 2);
}

void test_tile_cvtps2bf16() {
  // CHECK-LABEL: @test_tile_cvtps2bf16
  // CHECK: call void asm sideeffect "tcvtps2bf16 %tmm1, %tmm2
  _tile_cvtps2bf16(1, 2);
}

void test_tile_cvtps2bs() {
  // CHECK-LABEL: @test_tile_cvtps2bs
  // CHECK: call void asm sideeffect "tcvtps2bs %tmm1, %tmm2"
  _tile_cvtps2bs(1, 2);
}

void test_tile_cvtps2ubs() {
  // CHECK-LABEL: @test_tile_cvtps2ubs
  // CHECK: call void asm sideeffect "tcvtps2ubs %tmm1, %tmm2"
  _tile_cvtps2ubs(1, 2);
}

void test_tile_cvtub2ps() {
  // CHECK-LABEL: @test_tile_cvtub2ps
  // CHECK: call void asm sideeffect "tcvtub2ps %tmm1, %tmm2"
  _tile_cvtub2ps(1, 2);
}

void test_tile_fmaddps_reg() {
  // CHECK-LABEL: @test_tile_fmaddps_reg
  // CHECK: call void asm sideeffect "tfmaddps %tmm1, %tmm2, %tmm3"
  _tile_fmaddps_reg(1, 2, 3);
}

void test_tile_fmaddps_mem(void *A) {
  // CHECK-LABEL: @test_tile_fmaddps_mem
  // CHECK: call void asm sideeffect "tfmaddps $0, %tmm2, %tmm3"
  _tile_fmaddps_mem(A, 2, 3);
}

void test_tile_fmsubps_reg() {
  // CHECK-LABEL: @test_tile_fmsubps_reg
  // CHECK: call void asm sideeffect "tfmsubps %tmm1, %tmm2, %tmm3"
  _tile_fmsubps_reg(1, 2, 3);
}

void test_tile_fmsubps_mem(void *A) {
  // CHECK-LABEL: @test_tile_fmsubps_mem
  // CHECK: call void asm sideeffect "tfmsubps $0, %tmm2, %tmm3"
  _tile_fmsubps_mem(A, 2, 3);
}

void test_tile_fnmaddps_reg() {
  // CHECK-LABEL: @test_tile_fnmaddps_reg
  // CHECK: call void asm sideeffect "tfnmaddps %tmm1, %tmm2, %tmm3"
  _tile_fnmaddps_reg(1, 2, 3);
}

void test_tile_fnmaddps_mem(void *A) {
  // CHECK-LABEL: @test_tile_fnmaddps_mem
  // CHECK: call void asm sideeffect "tfnmaddps $0, %tmm2, %tmm3"
  _tile_fnmaddps_mem(A, 2, 3);
}

void test_tile_fnmsubps_reg() {
  // CHECK-LABEL: @test_tile_fnmsubps_reg
  // CHECK: call void asm sideeffect "tfnmsubps %tmm1, %tmm2, %tmm3"
  _tile_fnmsubps_reg(1, 2, 3);
}

void test_tile_fnmsubps_mem(void *A) {
  // CHECK-LABEL: @test_tile_fnmsubps_mem
  // CHECK: call void asm sideeffect "tfnmsubps $0, %tmm2, %tmm3"
  _tile_fnmsubps_mem(A, 2, 3);
}

void test_tile_maxps_reg() {
  // CHECK-LABEL: @test_tile_maxps_reg
  // CHECK: call void asm sideeffect "tmaxps %tmm1, %tmm2, %tmm3"
  _tile_maxps_reg(1, 2, 3);
}

void test_tile_maxps_mem(void *A) {
  // CHECK-LABEL: @test_tile_maxps_mem
  // CHECK: call void asm sideeffect "tmaxps $0, %tmm2, %tmm3"
  _tile_maxps_mem(A, 2, 3);
}

void test_tile_minps_reg() {
  // CHECK-LABEL: @test_tile_minps_reg
  // CHECK: call void asm sideeffect "tminps %tmm1, %tmm2, %tmm3"
  _tile_minps_reg(1, 2, 3);
}

void test_tile_minps_mem(void *A) {
  // CHECK-LABEL: @test_tile_minps_mem
  // CHECK: call void asm sideeffect "tminps $0, %tmm2, %tmm3"
  _tile_minps_mem(A, 2, 3);
}

void test_tile_mulps_reg() {
  // CHECK-LABEL: @test_tile_mulps_reg
  // CHECK: call void asm sideeffect "tmulps %tmm1, %tmm2, %tmm3"
  _tile_mulps_reg(1, 2, 3);
}

void test_tile_mulps_mem(void *A) {
  // CHECK-LABEL: @test_tile_mulps_mem
  // CHECK: call void asm sideeffect "tmulps $0, %tmm2, %tmm3"
  _tile_mulps_mem(A, 2, 3);
}

void test_tile_ord_reg() {
  // CHECK-LABEL: @test_tile_ord_reg
  // CHECK: call void asm sideeffect "tord %tmm1, %tmm2, %tmm3"
  _tile_ord_reg(1, 2, 3);
}

void test_tile_ord_mem(void *A) {
  // CHECK-LABEL: @test_tile_ord_mem
  // CHECK: call void asm sideeffect "tord $0, %tmm2, %tmm3"
  _tile_ord_mem(A, 2, 3);
}

void test_tile_rcp14ps() {
  // CHECK-LABEL: @test_tile_rcp14ps
  // CHECK: call void asm sideeffect "trcp14ps %tmm1, %tmm2"
  _tile_rcp14ps(1, 2);
}

void test_tile_reduceps() {
  // CHECK-LABEL: @test_tile_reduceps
  // CHECK: call void asm sideeffect "treduceps $0, %tmm1, %tmm2"
  _tile_reduceps(128, 1, 2);
}

void test_tile_scalefps_reg() {
  // CHECK-LABEL: @test_tile_scalefps_reg
  // CHECK: call void asm sideeffect "tscalefps %tmm1, %tmm2, %tmm3"
  _tile_scalefps_reg(1, 2, 3);
}

void test_tile_scalefps_mem(void *A) {
  // CHECK-LABEL: @test_tile_scalefps_mem
  // CHECK: call void asm sideeffect "tscalefps $0, %tmm2, %tmm3"
  _tile_scalefps_mem(A, 2, 3);
}

void test_tile_slld() {
  // CHECK-LABEL: @test_tile_slld
  // CHECK: call void asm sideeffect "tslld $0, %tmm1, %tmm2"
  _tile_slld(128, 1, 2);
}

void test_tile_srld() {
  // CHECK-LABEL: @test_tile_srld
  // CHECK: call void asm sideeffect "tsrld $0, %tmm1, %tmm2"
  _tile_srld(128, 1, 2);
}

void test_tile_srlvd_reg() {
  // CHECK-LABEL: @test_tile_srlvd_reg
  // CHECK: call void asm sideeffect "tsrlvd %tmm1, %tmm2, %tmm3"
  _tile_srlvd_reg(1, 2, 3);
}

void test_tile_srlvd_mem(void *A) {
  // CHECK-LABEL: @test_tile_srlvd_mem
  // CHECK: call void asm sideeffect "tsrlvd $0, %tmm2, %tmm3"
  _tile_srlvd_mem(A, 2, 3);
}

void test_tile_subps_reg() {
  // CHECK-LABEL: @test_tile_subps_reg
  // CHECK: call void asm sideeffect "tsubps %tmm1, %tmm2, %tmm3"
  _tile_subps_reg(1, 2, 3);
}

void test_tile_subps_mem(void *A) {
  // CHECK-LABEL: @test_tile_subps_mem
  // CHECK: call void asm sideeffect "tsubps $0, %tmm2, %tmm3"
  _tile_subps_mem(A, 2, 3);
}

void test_tile_xord_reg() {
  // CHECK-LABEL: @test_tile_xord_reg
  // CHECK: call void asm sideeffect "txord %tmm1, %tmm2, %tmm3"
  _tile_xord_reg(1, 2, 3);
}

void test_tile_xord_mem(void *A) {
  // CHECK-LABEL: @test_tile_xord_mem
  // CHECK: call void asm sideeffect "txord $0, %tmm2, %tmm3"
  _tile_xord_mem(A, 2, 3);
}
// FP16
void test_tile_dpfp16ps() {
  // CHECK-LABEL: @test_tile_dpfp16ps
  // CHECK: call void asm sideeffect "tdpfp16ps %tmm1, %tmm2, %tmm3"
  _tile_dpfp16ps(1, 2, 3);
}
// Tile to AVX512
__m512 test_tile_movrowe_imm() {
  // CHECK-LABEL: @test_tile_movrowe_imm
  // CHECK: call <16 x float> asm sideeffect "tilemovrowe $1, %tmm1, $0"
  // CHECK: ret <16 x float>
  return _tile_movrowe(1,4);
}

__m512 test_tile_movrowe_r32(__int32 row) {
  // CHECK-LABEL: @test_tile_movrowe_r32
  // CHECK: call <16 x float> asm sideeffect "tilemovrowe $1, %tmm1, $0"
  // CHECK: ret <16 x float>
  return _tile_movrowe(1,row);
}

__m512 test_tile_movrowe_xmm(__m128 row) {
  // CHECK-LABEL: @test_tile_movrowe_xmm
  // CHECK: call <16 x float> asm sideeffect "tilemovrowe $1, %tmm1, $0"
  // CHECK: ret <16 x float>
  return _tile_movrowe_x(1,row);
}

void test_tilemove() {
  // CHECK-LABEL: @test_tilemove
  // CHECK: call void asm sideeffect "tilemove %tmm13, %tmm1"
  _tile_move(1,13);
}

void test_tile16move() {
  // CHECK-LABEL: @test_tile16move
  // CHECK: call void asm sideeffect "tile16move %zmm13, %tmm1"
  _tile_16move(1,13);
}

void test_tileloadde(void *data){
  // CHECK-LABEL: @test_tileloadde
  // CHECK: call void asm sideeffect "tileloadde ($0,$1,1), %tmm4"
  _tile_loadde(4,data,8);
}

void test_tileloaddt1e(void *data){
  // CHECK-LABEL: @test_tileloaddt1e
  // CHECK: call void asm sideeffect "tileloaddt1e  ($0,$1,1), %tmm4"
  _tile_loaddt1e(4,data,8);
}

void test_tilestorede(void *data){
  // CHECK-LABEL: @test_tilestorede
  // CHECK: call void asm sideeffect "tilestorede  %tmm4, ($0,$1,1)"
  _tile_storede(4,data,8);
}

void test_tilezeroe(void *data){
  // CHECK-LABEL: @test_tilezeroe
  // CHECK: call void asm sideeffect "tilezeroe %tmm4"
  _tile_zeroe(4);
}

void test_tile_dpbf16pse() {
  // CHECK-LABEL: @test_tile_dpbf16pse
  //CHECK: call void asm sideeffect "tdpbf16pse %tmm3, %tmm2, %tmm1"
  _tile_dpbf16pse(1, 2, 3);
}

void test_tile_dpbssde() {
  // CHECK-LABEL: @test_tile_dpbssde
  //CHECK: call void asm sideeffect "tdpbssde %tmm3, %tmm2, %tmm1"
  _tile_dpbssde(1, 2, 3);
}

void test_tile_dpbsude() {
  // CHECK-LABEL: @test_tile_dpbsude
  //CHECK: call void asm sideeffect "tdpbsude %tmm3, %tmm2, %tmm1"
  _tile_dpbsude(1, 2, 3);
}

void test_tile_dpbusde() {
  // CHECK-LABEL: @test_tile_dpbusde
  //CHECK: call void asm sideeffect "tdpbusde %tmm3, %tmm2, %tmm1"
  _tile_dpbusde(1, 2, 3);
}

void test_tile_dpbuude() {
  // CHECK-LABEL: @test_tile_dpbuude
  //CHECK: call void asm sideeffect "tdpbuude %tmm3, %tmm2, %tmm1"
  _tile_dpbuude(1, 2, 3);
}
