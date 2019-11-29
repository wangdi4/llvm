// REQUIRES: intel_feature_isa_amx2
// RUN: %clang_cc1 %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-int8-evex -target-feature +amx-bf16-evex -target-feature +amx-tile-evex -target-feature\
// RUN: +amx-transpose -target-feature +amx-reduce -target-feature +amx-memory -target-feature +amx-format -target-feature +amx-element -target-feature +amx-fp16 \
// RUN: -target-feature +amx-avx512 -target-feature +avx512f -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// Transpose
void test_tile_2rpntlvw(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2rpntlvw
  // CHECK: call void @llvm.x86.t2rpntlvw(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2rpntlvw(1, A, B, C);
}

void test_tile_2rpntlvwt1(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2rpntlvwt1
  // CHECK: call void @llvm.x86.t2rpntlvwt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2rpntlvwt1(1, A, B, C);
}

void test_tile_2transposew(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposew
  // CHECK: call void @llvm.x86.t2transposew(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposew(1, A, B, C);
}

void test_tile_2transposewt1(const void *A, size_t B, size_t C) {
  // CHECK-LABEL: @test_tile_2transposewt1
  // CHECK: call void @llvm.x86.t2transposewt1(i8 1, i8* %{{.*}}, i64 %{{.*}}, i64 %{{.*}})
  _tile_2transposewt1(1, A, B, C);
}

// Reduce
void test_tile_coladdbcastps() {
  // CHECK-LABEL: @test_tile_coladdbcastps
  // CHECK: call void @llvm.x86.tcoladdbcastps(i8 1, i8 2)
  _tile_coladdbcastps(1, 2);
}

void test_tile_coladdps(void *A) {
  // CHECK-LABEL: @test_tile_coladdps
  // CHECK: call void @llvm.x86.tcoladdps(i8* %{{.*}}, i8 1)
  _tile_coladdps(A, 1);
}

// Memory
void test_tile_broadcastrowd(const void *A) {
  // CHECK-LABEL: @test_tile_broadcastrowd
  // CHECK: call void @llvm.x86.tbroadcastrowd(i8 1, i8* %{{.*}})
  _tile_broadcastrowd(1, A);
}

void test_tile_gatherrowd(const void *A, const void *B) {
  // CHECK-LABEL: @test_tile_gatherrowd
  // CHECK: call void @llvm.x86.tgatherrowd(i8 1, i8* %{{.*}}, i8* %{{.*}})
  _tile_gatherrowd(1, A, B);
}

void test_tile_gatherrowdt1(const void *A, const void *B) {
  // CHECK-LABEL: @test_tile_gatherrowdt1
  // CHECK: call void @llvm.x86.tgatherrowdt1(i8 1, i8* %{{.*}}, i8* %{{.*}})
  _tile_gatherrowdt1(1, A, B);
}

void test_tile_gatherrowq(const void *A, const void *B) {
  // CHECK-LABEL: @test_tile_gatherrowq
  // CHECK: call void @llvm.x86.tgatherrowq(i8 1, i8* %{{.*}}, i8* %{{.*}})
  _tile_gatherrowq(1, A, B);
}

void test_tile_gatherrowqt1(const void *A, const void *B) {
  // CHECK-LABEL: @test_tile_gatherrowqt1
  // CHECK: call void @llvm.x86.tgatherrowqt1(i8 1, i8* %{{.*}}, i8* %{{.*}})
  _tile_gatherrowqt1(1, A, B);
}

void test_tile_scatterrowd(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowd
  // CHECK: call void @llvm.x86.tscatterrowd(i8* %{{.*}}, i8* %{{.*}}, i8 1)
  _tile_scatterrowd(A, B, 1);
}

void test_tile_scatterrowdt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowdt1
  // CHECK: call void @llvm.x86.tscatterrowdt1(i8* %{{.*}}, i8* %{{.*}}, i8 1)
  _tile_scatterrowdt1(A, B, 1);
}

void test_tile_scatterrowq(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowq
  // CHECK: call void @llvm.x86.tscatterrowq(i8* %{{.*}}, i8* %{{.*}}, i8 1)
  _tile_scatterrowq(A, B, 1);
}

void test_tile_scatterrowqt1(void *A, void *B) {
  // CHECK-LABEL: @test_tile_scatterrowqt1
  // CHECK: call void @llvm.x86.tscatterrowqt1(i8* %{{.*}}, i8* %{{.*}}, i8 1)
  _tile_scatterrowqt1(A, B, 1);
}

void test_tile_storehd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storehd
  // CHECK: call void @llvm.x86.tstorehd(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storehd(A, B, 1);
}

void test_tile_storehdt1(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storehdt1
  // CHECK: call void @llvm.x86.tstorehdt1(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storehdt1(A, B, 1);
}

void test_tile_storentd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storentd
  // CHECK: call void @llvm.x86.tstorentd(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storentd(A, B, 1);
}

void test_tile_storeqd(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storeqd
  // CHECK: call void @llvm.x86.tstoreqd(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storeqd(A, B, 1);
}

void test_tile_storeqdt1(void *A, size_t B) {
  // CHECK-LABEL: @test_tile_storeqdt1
  // CHECK: call void @llvm.x86.tstoreqdt1(i8* %{{.*}}, i64 %{{.*}}, i8 1)
  _tile_storeqdt1(A, B, 1);
}

void test_tile_storerowd(void *A) {
  // CHECK-LABEL: @test_tile_storerowd
  // CHECK: call void @llvm.x86.tstorerowd(i8* %{{.*}}, i8 1)
  _tile_storerowd(A, 1);
}

// Format
void test_tile_blendvd() {
  // CHECK-LABEL: @test_tile_blendvd
  // CHECK: call void @llvm.x86.tblendvd(i8 1, i8 2, i8 3)
  _tile_blendvd(1, 2, 3);
}

void test_tile_interleaveeb() {
  // CHECK-LABEL: @test_tile_interleaveeb
  // CHECK: call void @llvm.x86.tinterleaveeb(i8 1, i8 2, i8 3)
  _tile_interleaveeb(1, 2, 3);
}

void test_tile_interleaveew() {
  // CHECK-LABEL: @test_tile_interleaveew
  // CHECK: call void @llvm.x86.tinterleaveew(i8 1, i8 2, i8 3)
  _tile_interleaveew(1, 2, 3);
}

void test_tile_interleaveob() {
  // CHECK-LABEL: @test_tile_interleaveob
  // CHECK: call void @llvm.x86.tinterleaveob(i8 1, i8 2, i8 3)
  _tile_interleaveob(1, 2, 3);
}

void test_tile_interleaveow() {
  // CHECK-LABEL: @test_tile_interleaveow
  // CHECK: call void @llvm.x86.tinterleaveow(i8 1, i8 2, i8 3)
  _tile_interleaveow(1, 2, 3);
}

void test_tile_narrowb() {
  // CHECK-LABEL: @test_tile_narrowb
  // CHECK: call void @llvm.x86.tnarrowb(i8 1, i8 2, i8 127)
  _tile_narrowb(1, 2, 127);
}

void test_tile_narroww() {
  // CHECK-LABEL: @test_tile_narroww
  // CHECK: call void @llvm.x86.tnarroww(i8 1, i8 2, i8 127)
  _tile_narroww(1, 2, 127);
}

void test_tile_permb_reg() {
  // CHECK-LABEL: @test_tile_permb_reg
  // CHECK: call void @llvm.x86.tpermb.reg(i8 1, i8 2, i8 3)
  _tile_permb_reg(1, 2, 3);
}

void test_tile_permb_mem(const void *A) {
  // CHECK-LABEL: @test_tile_permb_mem
  // CHECK: call void @llvm.x86.tpermb.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permb_mem(1, 2, A);
}

void test_tile_permd_reg() {
  // CHECK-LABEL: @test_tile_permd_reg
  // CHECK: call void @llvm.x86.tpermd.reg(i8 1, i8 2, i8 3)
  _tile_permd_reg(1, 2, 3);
}

void test_tile_permd_mem(const void *A) {
  //  CHECK-LABEL: @test_tile_permd_mem
  // CHECK: call void @llvm.x86.tpermd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permd_mem(1, 2, A);
}

void test_tile_permw_reg() {
  // CHECK-LABEL: @test_tile_permw_reg
  // CHECK: call void @llvm.x86.tpermw.reg(i8 1, i8 2, i8 3)
  _tile_permw_reg(1, 2, 3);
}

void test_tile_permw_mem(const void *A) {
  // CHECK-LABEL: @test_tile_permw_mem
  // CHECK: call void @llvm.x86.tpermw.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permw_mem(1, 2, A);
}

void test_tile_widenb() {
  // CHECK-LABEL: @test_tile_widenb
  // CHECK: call void @llvm.x86.twidenb(i8 1, i8 2, i8 127)
  _tile_widenb(1, 2, 127);
}

void test_tile_widenw() {
  // CHECK-LABEL: @test_tile_widenw
  // CHECK: call void @llvm.x86.twidenw(i8 1, i8 2, i8 127)
  _tile_widenw(1, 2, 127);
}

// Element
void test_tile_addps_reg() {
  // CHECK-LABEL: @test_tile_addps_reg
  // CHECK: call void @llvm.x86.taddps.reg(i8 1, i8 2, i8 3)
  _tile_addps_reg(1, 2, 3);
}

void test_tile_addps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_addps_mem
  // CHECK: call void @llvm.x86.taddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_addps_mem(1, 2, A);
}

void test_tile_andd_reg() {
  // CHECK-LABEL: @test_tile_andd_reg
  // CHECK: call void @llvm.x86.tandd.reg(i8 1, i8 2, i8 3)
  _tile_andd_reg(1, 2, 3);
}

void test_tile_andd_mem(const void *A) {
  // CHECK-LABEL: @test_tile_andd_mem
  // CHECK: call void @llvm.x86.tandd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_andd_mem(1, 2, A);
}

void test_tile_andnd_reg() {
  // CHECK-LABEL: @test_tile_andnd_reg
  // CHECK: call void @llvm.x86.tandnd.reg(i8 1, i8 2, i8 3)
  _tile_andnd_reg(1, 2, 3);
}

void test_tile_andnd_mem(const void *A) {
  // CHECK-LABEL: @test_tile_andnd_mem
  // CHECK: call void @llvm.x86.tandnd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_andnd_mem(1, 2, A);
}

void test_tile_cmpps_reg() {
  // CHECK-LABEL: @test_tile_cmpps_reg
  // CHECK: call void @llvm.x86.tcmpps.reg(i8 1, i8 2, i8 3, i8 127)
  _tile_cmpps_reg(1, 2, 3, 127);
}

void test_tile_cmpps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_cmpps_mem
  // CHECK: call void @llvm.x86.tcmpps.mem(i8 1, i8 2, i8* {{.*}}, i8 127)
  _tile_cmpps_mem(1, 2, A, 127);
}

void test_tile_cvtb2ps() {
  // CHECK-LABEL: @test_tile_cvtb2ps
  // CHECK: call void @llvm.x86.tcvtb2ps(i8 1, i8 2)
  _tile_cvtb2ps(1, 2);
}

void test_tile_cvtbf162ps() {
  // CHECK-LABEL: @test_tile_cvtbf162ps
  // CHECK: call void @llvm.x86.tcvtbf162ps(i8 1, i8 2)
  _tile_cvtbf162ps(1, 2);
}

void test_tile_cvtd2ps() {
  // CHECK-LABEL: @test_tile_cvtd2ps
  // CHECK: call void @llvm.x86.tcvtd2ps(i8 1, i8 2)
  _tile_cvtd2ps(1, 2);
}

void test_tile_cvtps2bf16() {
  // CHECK-LABEL: @test_tile_cvtps2bf16
  // CHECK: call void @llvm.x86.tcvtps2bf16(i8 1, i8 2)
  _tile_cvtps2bf16(1, 2);
}

void test_tile_cvtps2bs() {
  // CHECK-LABEL: @test_tile_cvtps2bs
  // CHECK: call void @llvm.x86.tcvtps2bs(i8 1, i8 2)
  _tile_cvtps2bs(1, 2);
}

void test_tile_cvtps2ubs() {
  // CHECK-LABEL: @test_tile_cvtps2ubs
  // CHECK: call void @llvm.x86.tcvtps2ubs(i8 1, i8 2)
  _tile_cvtps2ubs(1, 2);
}

void test_tile_cvtub2ps() {
  // CHECK-LABEL: @test_tile_cvtub2ps
  // CHECK: call void @llvm.x86.tcvtub2ps(i8 1, i8 2)
  _tile_cvtub2ps(1, 2);
}

void test_tile_fmaddps_reg() {
  // CHECK-LABEL: @test_tile_fmaddps_reg
  // CHECK: call void @llvm.x86.tfmaddps.reg(i8 1, i8 2, i8 3)
  _tile_fmaddps_reg(1, 2, 3);
}

void test_tile_fmaddps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fmaddps_mem
  // CHECK: call void @llvm.x86.tfmaddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fmaddps_mem(1, 2, A);
}

void test_tile_fmsubps_reg() {
  // CHECK-LABEL: @test_tile_fmsubps_reg
  // CHECK: call void @llvm.x86.tfmsubps.reg(i8 1, i8 2, i8 3)
  _tile_fmsubps_reg(1, 2, 3);
}

void test_tile_fmsubps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fmsubps_mem
  // CHECK: call void @llvm.x86.tfmsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fmsubps_mem(1, 2, A);
}

void test_tile_fnmaddps_reg() {
  // CHECK-LABEL: @test_tile_fnmaddps_reg
  // CHECK: call void @llvm.x86.tfnmaddps.reg(i8 1, i8 2, i8 3)
  _tile_fnmaddps_reg(1, 2, 3);
}

void test_tile_fnmaddps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fnmaddps_mem
  // CHECK: call void @llvm.x86.tfnmaddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fnmaddps_mem(1, 2, A);
}

void test_tile_fnmsubps_reg() {
  // CHECK-LABEL: @test_tile_fnmsubps_reg
  // CHECK: call void @llvm.x86.tfnmsubps.reg(i8 1, i8 2, i8 3)
  _tile_fnmsubps_reg(1, 2, 3);
}

void test_tile_fnmsubps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fnmsubps_mem
  // CHECK: call void @llvm.x86.tfnmsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fnmsubps_mem(1, 2, A);
}

void test_tile_maxps_reg() {
  // CHECK-LABEL: @test_tile_maxps_reg
  // CHECK: call void @llvm.x86.tmaxps.reg(i8 1, i8 2, i8 3)
  _tile_maxps_reg(1, 2, 3);
}

void test_tile_maxps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_maxps_mem
  // CHECK: call void @llvm.x86.tmaxps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_maxps_mem(1, 2, A);
}

void test_tile_minps_reg() {
  // CHECK-LABEL: @test_tile_minps_reg
  // CHECK: call void @llvm.x86.tminps.reg(i8 1, i8 2, i8 3)
  _tile_minps_reg(1, 2, 3);
}

void test_tile_minps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_minps_mem
  // CHECK: call void @llvm.x86.tminps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_minps_mem(1, 2, A);
}

void test_tile_mulps_reg() {
  // CHECK-LABEL: @test_tile_mulps_reg
  // CHECK: call void @llvm.x86.tmulps.reg(i8 1, i8 2, i8 3)
  _tile_mulps_reg(1, 2, 3);
}

void test_tile_mulps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_mulps_mem
  // CHECK: call void @llvm.x86.tmulps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_mulps_mem(1, 2, A);
}

void test_tile_ord_reg() {
  // CHECK-LABEL: @test_tile_ord_reg
  // CHECK: call void @llvm.x86.tord.reg(i8 1, i8 2, i8 3)
  _tile_ord_reg(1, 2, 3);
}

void test_tile_ord_mem(const void *A) {
  // CHECK-LABEL: @test_tile_ord_mem
  // CHECK: call void @llvm.x86.tord.mem(i8 1, i8 2, i8* {{.*}})
  _tile_ord_mem(1, 2, A);
}

void test_tile_rcp14ps() {
  // CHECK-LABEL: @test_tile_rcp14ps
  // CHECK: call void @llvm.x86.trcp14ps(i8 1, i8 2)
  _tile_rcp14ps(1, 2);
}

void test_tile_reduceps() {
  // CHECK-LABEL: @test_tile_reduceps
  // CHECK: call void @llvm.x86.treduceps(i8 1, i8 2, i8 127)
  _tile_reduceps(1, 2, 127);
}

void test_tile_scalefps_reg() {
  // CHECK-LABEL: @test_tile_scalefps_reg
  // CHECK: call void @llvm.x86.tscalefps.reg(i8 1, i8 2, i8 3)
  _tile_scalefps_reg(1, 2, 3);
}

void test_tile_scalefps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_scalefps_mem
  // CHECK: call void @llvm.x86.tscalefps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_scalefps_mem(1, 2, A);
}

void test_tile_slld() {
  // CHECK-LABEL: @test_tile_slld
  // CHECK: call void @llvm.x86.tslld(i8 1, i8 2, i8 127)
  _tile_slld(1, 2, 127);
}

void test_tile_srld() {
  // CHECK-LABEL: @test_tile_srld
  // CHECK: call void @llvm.x86.tsrld(i8 1, i8 2, i8 127)
  _tile_srld(1, 2, 127);
}

void test_tile_srlvd_reg() {
  // CHECK-LABEL: @test_tile_srlvd_reg
  // CHECK: call void @llvm.x86.tsrlvd.reg(i8 1, i8 2, i8 3)
  _tile_srlvd_reg(1, 2, 3);
}

void test_tile_srlvd_mem(void *A) {
  // CHECK-LABEL: @test_tile_srlvd_mem
  // CHECK: call void @llvm.x86.tsrlvd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_srlvd_mem(1, 2, A);
}

void test_tile_subps_reg() {
  // CHECK-LABEL: @test_tile_subps_reg
  // CHECK: call void @llvm.x86.tsubps.reg(i8 1, i8 2, i8 3)
  _tile_subps_reg(1, 2, 3);
}

void test_tile_subps_mem(void *A) {
  // CHECK-LABEL: @test_tile_subps_mem
  // CHECK: call void @llvm.x86.tsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_subps_mem(1, 2, A);
}

void test_tile_xord_reg() {
  // CHECK-LABEL: @test_tile_xord_reg
  // CHECK: call void @llvm.x86.txord.reg(i8 1, i8 2, i8 3)
  _tile_xord_reg(1, 2, 3);
}

void test_tile_xord_mem(void *A) {
  // CHECK-LABEL: @test_tile_xord_mem
  // CHECK: call void @llvm.x86.txord.mem(i8 1, i8 2, i8* {{.*}})
  _tile_xord_mem(1, 2, A);
}

// FP16
void test_tile_dpfp16ps() {
  // CHECK-LABEL: @test_tile_dpfp16ps
  // CHECK: call void @llvm.x86.tdpfp16ps(i8 1, i8 2, i8 3)
  _tile_dpfp16ps(1, 2, 3);
}

typedef float __m512 __attribute__((__vector_size__(64)));
// Tile to AVX512
void test_tile_mov16zmm(__m512 tsrc1, __m512 tsrc2, __m512 tsrc3, __m512 tsrc4,
  __m512 tsrc5, __m512 tsrc6, __m512 tsrc7, __m512 tsrc8, __m512 tsrc9,
  __m512 tsrc10, __m512 tsrc11, __m512 tsrc12, __m512 tsrc13, __m512 tsrc14,
  __m512 tsrc15, __m512 tsrc16) {
  // CHECK-LABEL: @test_tile_mov16zmm
  // CHECK: call void @llvm.x86.tile16move(i8 1, <16 x float> %0,{{.*}}<16 x float> %15
  _tile_tile16move(1, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7, tsrc8,
              tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16);
}

void test_tile_tilemovrowei() {
  // CHECK-LABEL: @test_tile_tilemovrowei
  // CHECK: %0 = call <16 x float> @llvm.x86.tilemovei(i8 1, i8 2)
  _tile_tilemovrowei(1, 2);
}

typedef unsigned int uint32_t;
void test_tile_tilemovrowee(uint32_t A) {
  // CHECK-LABEL: @test_tile_tilemovrowee
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovee(i8 1, i32 %0)
  _tile_tilemovrowee(1, A);
}

typedef float __m128 __attribute__((__vector_size__(16)));
void test_tile_tilemovrowex(__m128 A) {
  // CHECK-LABEL: @test_tile_tilemovrowex
  // CHECK: %1 = call <16 x float> @llvm.x86.tilemovex(i8 1, <4 x float> %0)
  _tile_tilemovrowex(1, A);
}

// BF16-EVEX
void test_tile_dpbf16pse() {
  // CHECK-LABEL: @test_tile_dpbf16pse
  // CHECK: call void @llvm.x86.tdpbf16pse(i8 1, i8 2, i8 3)
  _tile_dpbf16pse(1, 2, 3);
}

// INT8-EVEX
void test_tile_tdpbssde() {
  // CHECK-LABEL: @test_tile_tdpbssde
  // CHECK: call void @llvm.x86.tdpbssde(i8 1, i8 2, i8 3)
  _tile_dpbssde(1, 2, 3);
}

void test_tile_tdpbsude() {
  // CHECK-LABEL: @test_tile_tdpbsude
  // CHECK: call void @llvm.x86.tdpbsude(i8 1, i8 2, i8 3)
  _tile_dpbsude(1, 2, 3);
}

void test_tile_tdpbusde() {
  // CHECK-LABEL: @test_tile_tdpbusde
  // CHECK: call void @llvm.x86.tdpbusde(i8 1, i8 2, i8 3)
  _tile_dpbusde(1, 2, 3);
}

void test_tile_tdpbuude() {
  // CHECK-LABEL: @test_tile_tdpbuude
  // CHECK: call void @llvm.x86.tdpbuude(i8 1, i8 2, i8 3)
  _tile_dpbuude(1, 2, 3);
}

// TILE-EVEX
void test_tile_loadde(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_loadde
  // CHECK: call void @llvm.x86.tileloadde64(i8 1, i8* %0, i64 %1)
  _tile_loadde(1, base, stride);
}

void test_tile_tileloaddt164e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tileloaddt164e
  // CHECK: call void @llvm.x86.tileloaddt1e64(i8 1, i8* %0, i64 %1)
  _tile_stream_loadde(1, base, stride);
}

void test_tile_tilestored64e(const void * base, size_t stride) {
  // CHECK-LABEL: @test_tile_tilestored64e
  // CHECK: call void @llvm.x86.tilestorede64(i8 1, i8* %0, i64 %1)
  _tile_storede(1, base, stride);
}

void test_tile_tilemove() {
  // CHECK-LABEL: @test_tile_tilemove
  // CHECK: call void @llvm.x86.tilemove(i8 1, i8 2)
  _tile_tilemove(1, 2);
}

void test_tile_tilezeroe() {
  // CHECK-LABEL: @test_tile_tilezeroe
  // CHECK: call void @llvm.x86.tilezeroe(i8 1)
  _tile_zeroe(1);
}
