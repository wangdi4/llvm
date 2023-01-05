; REQUIRES: intel_feature_isa_amx_avx512_tile16mov
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-bf16,+amx-avx512-tile16mov | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    tile16move %zmm{{.*}}, %tmm4

define void @test_amx(i64 %addr, i64 %addrx, i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1,i8* %addr2, <16 x float> %zmm, <4 x float> %xmm) {
; amx-avx512
call void @llvm.x86.tile16move(i8 4, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-avx512
declare void @llvm.x86.tile16move(i8 %tile0, <16 x float> %zmm0, <16 x float> %zmm1, <16 x float> %zmm2, <16 x float> %zmm3, <16 x float> %zmm4, <16 x float> %zmm5, <16 x float> %zmm6, <16 x float> %zmm7, <16 x float> %zmm8, <16 x float> %zmm9, <16 x float> %zmm10, <16 x float> %zmm11, <16 x float> %zmm12, <16 x float> %zmm13, <16 x float> %zmm14, <16 x float> %zmm15)
