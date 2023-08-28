// REQUIRES: intel_feature_isa_amx_future
// RUN: %clang_cc1 -no-opaque-pointers %s -ffreestanding -triple=x86_64-unknown-unknown \
// RUN: -target-feature +amx-int8 -target-feature +amx-bf16 -target-feature +amx-reduce -target-feature +amx-memory  \
// RUN: -target-feature +amx-format -target-feature +amx-element  -emit-llvm -o - -Wall -Werror -pedantic \
// RUN: -Wno-gnu-statement-expression| FileCheck %s

#include <immintrin.h>
#include <stddef.h>
// Reduce
void test_tile_coladdbcastps(void) {
  // CHECK-LABEL: @test_tile_coladdbcastps
  // CHECK: call void @llvm.x86.tcoladdbcastps(i8 1, i8 2)
  _tile_coladdbcastps(1, 2);
}

void test_tile_coladdps(void *A) {
  // CHECK-LABEL: @test_tile_coladdps
  // CHECK: call void @llvm.x86.tcoladdps(i8* %{{.*}}, i8 1)
  _tile_coladdps(A, 1);
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

// Format
void test_tile_blendvd(void) {
  // CHECK-LABEL: @test_tile_blendvd
  // CHECK: call void @llvm.x86.tblendvd(i8 1, i8 2, i8 3)
  _tile_blendvd(1, 2, 3);
}

void test_tile_interleaveeb(void) {
  // CHECK-LABEL: @test_tile_interleaveeb
  // CHECK: call void @llvm.x86.tinterleaveeb(i8 1, i8 2, i8 3)
  _tile_interleaveeb(1, 2, 3);
}

void test_tile_interleaveew(void) {
  // CHECK-LABEL: @test_tile_interleaveew
  // CHECK: call void @llvm.x86.tinterleaveew(i8 1, i8 2, i8 3)
  _tile_interleaveew(1, 2, 3);
}

void test_tile_interleaveob(void) {
  // CHECK-LABEL: @test_tile_interleaveob
  // CHECK: call void @llvm.x86.tinterleaveob(i8 1, i8 2, i8 3)
  _tile_interleaveob(1, 2, 3);
}

void test_tile_interleaveow(void) {
  // CHECK-LABEL: @test_tile_interleaveow
  // CHECK: call void @llvm.x86.tinterleaveow(i8 1, i8 2, i8 3)
  _tile_interleaveow(1, 2, 3);
}

void test_tile_narrowb(void) {
  // CHECK-LABEL: @test_tile_narrowb
  // CHECK: call void @llvm.x86.tnarrowb(i8 1, i8 2, i8 127)
  _tile_narrowb(1, 2, 127);
}

void test_tile_narroww(void) {
  // CHECK-LABEL: @test_tile_narroww
  // CHECK: call void @llvm.x86.tnarroww(i8 1, i8 2, i8 127)
  _tile_narroww(1, 2, 127);
}

void test_tile_permb_reg(void) {
  // CHECK-LABEL: @test_tile_permb_reg
  // CHECK: call void @llvm.x86.tpermb.reg(i8 1, i8 2, i8 3)
  _tile_permb_reg(1, 2, 3);
}

void test_tile_permb_mem(const void *A) {
  // CHECK-LABEL: @test_tile_permb_mem
  // CHECK: call void @llvm.x86.tpermb.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permb_mem(1, 2, A);
}

void test_tile_permd_reg(void) {
  // CHECK-LABEL: @test_tile_permd_reg
  // CHECK: call void @llvm.x86.tpermd.reg(i8 1, i8 2, i8 3)
  _tile_permd_reg(1, 2, 3);
}

void test_tile_permd_mem(const void *A) {
  //  CHECK-LABEL: @test_tile_permd_mem
  // CHECK: call void @llvm.x86.tpermd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permd_mem(1, 2, A);
}

void test_tile_permw_reg(void) {
  // CHECK-LABEL: @test_tile_permw_reg
  // CHECK: call void @llvm.x86.tpermw.reg(i8 1, i8 2, i8 3)
  _tile_permw_reg(1, 2, 3);
}

void test_tile_permw_mem(const void *A) {
  // CHECK-LABEL: @test_tile_permw_mem
  // CHECK: call void @llvm.x86.tpermw.mem(i8 1, i8 2, i8* {{.*}})
  _tile_permw_mem(1, 2, A);
}

void test_tile_widenb(void) {
  // CHECK-LABEL: @test_tile_widenb
  // CHECK: call void @llvm.x86.twidenb(i8 1, i8 2, i8 127)
  _tile_widenb(1, 2, 127);
}

void test_tile_widenw(void) {
  // CHECK-LABEL: @test_tile_widenw
  // CHECK: call void @llvm.x86.twidenw(i8 1, i8 2, i8 127)
  _tile_widenw(1, 2, 127);
}

// Element
void test_tile_addps_reg(void) {
  // CHECK-LABEL: @test_tile_addps_reg
  // CHECK: call void @llvm.x86.taddps.reg(i8 1, i8 2, i8 3)
  _tile_addps_reg(1, 2, 3);
}

void test_tile_addps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_addps_mem
  // CHECK: call void @llvm.x86.taddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_addps_mem(1, 2, A);
}

void test_tile_andd_reg(void) {
  // CHECK-LABEL: @test_tile_andd_reg
  // CHECK: call void @llvm.x86.tandd.reg(i8 1, i8 2, i8 3)
  _tile_andd_reg(1, 2, 3);
}

void test_tile_andd_mem(const void *A) {
  // CHECK-LABEL: @test_tile_andd_mem
  // CHECK: call void @llvm.x86.tandd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_andd_mem(1, 2, A);
}

void test_tile_andnd_reg(void) {
  // CHECK-LABEL: @test_tile_andnd_reg
  // CHECK: call void @llvm.x86.tandnd.reg(i8 1, i8 2, i8 3)
  _tile_andnd_reg(1, 2, 3);
}

void test_tile_andnd_mem(const void *A) {
  // CHECK-LABEL: @test_tile_andnd_mem
  // CHECK: call void @llvm.x86.tandnd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_andnd_mem(1, 2, A);
}

void test_tile_cmpps_reg(void) {
  // CHECK-LABEL: @test_tile_cmpps_reg
  // CHECK: call void @llvm.x86.tcmpps.reg(i8 1, i8 2, i8 3, i8 127)
  _tile_cmpps_reg(1, 2, 3, 127);
}

void test_tile_cmpps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_cmpps_mem
  // CHECK: call void @llvm.x86.tcmpps.mem(i8 1, i8 2, i8* {{.*}}, i8 127)
  _tile_cmpps_mem(1, 2, A, 127);
}

void test_tile_cvtb2ps(void) {
  // CHECK-LABEL: @test_tile_cvtb2ps
  // CHECK: call void @llvm.x86.tcvtb2ps(i8 1, i8 2)
  _tile_cvtb2ps(1, 2);
}

void test_tile_cvtbf162ps(void) {
  // CHECK-LABEL: @test_tile_cvtbf162ps
  // CHECK: call void @llvm.x86.tcvtbf162ps(i8 1, i8 2)
  _tile_cvtbf162ps(1, 2);
}

void test_tile_cvtd2ps(void) {
  // CHECK-LABEL: @test_tile_cvtd2ps
  // CHECK: call void @llvm.x86.tcvtd2ps(i8 1, i8 2)
  _tile_cvtd2ps(1, 2);
}

void test_tile_cvtps2bf16(void) {
  // CHECK-LABEL: @test_tile_cvtps2bf16
  // CHECK: call void @llvm.x86.tcvtps2bf16(i8 1, i8 2)
  _tile_cvtps2bf16(1, 2);
}

void test_tile_cvtps2bs(void) {
  // CHECK-LABEL: @test_tile_cvtps2bs
  // CHECK: call void @llvm.x86.tcvtps2bs(i8 1, i8 2)
  _tile_cvtps2bs(1, 2);
}

void test_tile_cvtps2ubs(void) {
  // CHECK-LABEL: @test_tile_cvtps2ubs
  // CHECK: call void @llvm.x86.tcvtps2ubs(i8 1, i8 2)
  _tile_cvtps2ubs(1, 2);
}

void test_tile_cvtub2ps(void) {
  // CHECK-LABEL: @test_tile_cvtub2ps
  // CHECK: call void @llvm.x86.tcvtub2ps(i8 1, i8 2)
  _tile_cvtub2ps(1, 2);
}

void test_tile_fmaddps_reg(void) {
  // CHECK-LABEL: @test_tile_fmaddps_reg
  // CHECK: call void @llvm.x86.tfmaddps.reg(i8 1, i8 2, i8 3)
  _tile_fmaddps_reg(1, 2, 3);
}

void test_tile_fmaddps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fmaddps_mem
  // CHECK: call void @llvm.x86.tfmaddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fmaddps_mem(1, 2, A);
}

void test_tile_fmsubps_reg(void) {
  // CHECK-LABEL: @test_tile_fmsubps_reg
  // CHECK: call void @llvm.x86.tfmsubps.reg(i8 1, i8 2, i8 3)
  _tile_fmsubps_reg(1, 2, 3);
}

void test_tile_fmsubps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fmsubps_mem
  // CHECK: call void @llvm.x86.tfmsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fmsubps_mem(1, 2, A);
}

void test_tile_fnmaddps_reg(void) {
  // CHECK-LABEL: @test_tile_fnmaddps_reg
  // CHECK: call void @llvm.x86.tfnmaddps.reg(i8 1, i8 2, i8 3)
  _tile_fnmaddps_reg(1, 2, 3);
}

void test_tile_fnmaddps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fnmaddps_mem
  // CHECK: call void @llvm.x86.tfnmaddps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fnmaddps_mem(1, 2, A);
}

void test_tile_fnmsubps_reg(void) {
  // CHECK-LABEL: @test_tile_fnmsubps_reg
  // CHECK: call void @llvm.x86.tfnmsubps.reg(i8 1, i8 2, i8 3)
  _tile_fnmsubps_reg(1, 2, 3);
}

void test_tile_fnmsubps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_fnmsubps_mem
  // CHECK: call void @llvm.x86.tfnmsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_fnmsubps_mem(1, 2, A);
}

void test_tile_maxps_reg(void) {
  // CHECK-LABEL: @test_tile_maxps_reg
  // CHECK: call void @llvm.x86.tmaxps.reg(i8 1, i8 2, i8 3)
  _tile_maxps_reg(1, 2, 3);
}

void test_tile_maxps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_maxps_mem
  // CHECK: call void @llvm.x86.tmaxps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_maxps_mem(1, 2, A);
}

void test_tile_minps_reg(void) {
  // CHECK-LABEL: @test_tile_minps_reg
  // CHECK: call void @llvm.x86.tminps.reg(i8 1, i8 2, i8 3)
  _tile_minps_reg(1, 2, 3);
}

void test_tile_minps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_minps_mem
  // CHECK: call void @llvm.x86.tminps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_minps_mem(1, 2, A);
}

void test_tile_mulps_reg(void) {
  // CHECK-LABEL: @test_tile_mulps_reg
  // CHECK: call void @llvm.x86.tmulps.reg(i8 1, i8 2, i8 3)
  _tile_mulps_reg(1, 2, 3);
}

void test_tile_mulps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_mulps_mem
  // CHECK: call void @llvm.x86.tmulps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_mulps_mem(1, 2, A);
}

void test_tile_ord_reg(void) {
  // CHECK-LABEL: @test_tile_ord_reg
  // CHECK: call void @llvm.x86.tord.reg(i8 1, i8 2, i8 3)
  _tile_ord_reg(1, 2, 3);
}

void test_tile_ord_mem(const void *A) {
  // CHECK-LABEL: @test_tile_ord_mem
  // CHECK: call void @llvm.x86.tord.mem(i8 1, i8 2, i8* {{.*}})
  _tile_ord_mem(1, 2, A);
}

void test_tile_rcp14ps(void) {
  // CHECK-LABEL: @test_tile_rcp14ps
  // CHECK: call void @llvm.x86.trcp14ps(i8 1, i8 2)
  _tile_rcp14ps(1, 2);
}

void test_tile_reduceps(void) {
  // CHECK-LABEL: @test_tile_reduceps
  // CHECK: call void @llvm.x86.treduceps(i8 1, i8 2, i8 127)
  _tile_reduceps(1, 2, 127);
}

void test_tile_scalefps_reg(void) {
  // CHECK-LABEL: @test_tile_scalefps_reg
  // CHECK: call void @llvm.x86.tscalefps.reg(i8 1, i8 2, i8 3)
  _tile_scalefps_reg(1, 2, 3);
}

void test_tile_scalefps_mem(const void *A) {
  // CHECK-LABEL: @test_tile_scalefps_mem
  // CHECK: call void @llvm.x86.tscalefps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_scalefps_mem(1, 2, A);
}

void test_tile_slld(void) {
  // CHECK-LABEL: @test_tile_slld
  // CHECK: call void @llvm.x86.tslld(i8 1, i8 2, i8 127)
  _tile_slld(1, 2, 127);
}

void test_tile_srld(void) {
  // CHECK-LABEL: @test_tile_srld
  // CHECK: call void @llvm.x86.tsrld(i8 1, i8 2, i8 127)
  _tile_srld(1, 2, 127);
}

void test_tile_srlvd_reg(void) {
  // CHECK-LABEL: @test_tile_srlvd_reg
  // CHECK: call void @llvm.x86.tsrlvd.reg(i8 1, i8 2, i8 3)
  _tile_srlvd_reg(1, 2, 3);
}

void test_tile_srlvd_mem(void *A) {
  // CHECK-LABEL: @test_tile_srlvd_mem
  // CHECK: call void @llvm.x86.tsrlvd.mem(i8 1, i8 2, i8* {{.*}})
  _tile_srlvd_mem(1, 2, A);
}

void test_tile_subps_reg(void) {
  // CHECK-LABEL: @test_tile_subps_reg
  // CHECK: call void @llvm.x86.tsubps.reg(i8 1, i8 2, i8 3)
  _tile_subps_reg(1, 2, 3);
}

void test_tile_subps_mem(void *A) {
  // CHECK-LABEL: @test_tile_subps_mem
  // CHECK: call void @llvm.x86.tsubps.mem(i8 1, i8 2, i8* {{.*}})
  _tile_subps_mem(1, 2, A);
}

void test_tile_xord_reg(void) {
  // CHECK-LABEL: @test_tile_xord_reg
  // CHECK: call void @llvm.x86.txord.reg(i8 1, i8 2, i8 3)
  _tile_xord_reg(1, 2, 3);
}

void test_tile_xord_mem(void *A) {
  // CHECK-LABEL: @test_tile_xord_mem
  // CHECK: call void @llvm.x86.txord.mem(i8 1, i8 2, i8* {{.*}})
  _tile_xord_mem(1, 2, A);
}
