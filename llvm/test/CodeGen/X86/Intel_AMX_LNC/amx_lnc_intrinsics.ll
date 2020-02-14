; REQUIRES: intel_feature_isa_amx_lnc
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-bf16,+amx-int8,+amx-transpose,+amx-avx512,+amx-tile-evex,+amx-int8-evex,+amx-bf16-evex,+amx-memory,+amx-element-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-avx512
; CHECK:    tcvtrowd2pse $9, %tmm7, %zmm{{.*}}
; CHECK:    tcvtrowd2pse %{{.*}}, %tmm7, %zmm{{.*}}
; CHECK:    tile16move %zmm{{.*}}, %tmm4
; CHECK:    tilemovrowe $9, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrowe %{{.*}}, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrowe %xmm{{.*}}, %tmm7, %zmm{{.*}}
; amx-bf16-evex
; CHECK:    tdpbf16pse %tmm7, %tmm4, %tmm3
; amx-element-evex
; CHECK:    tcvtd2pse        %tmm1, (%{{.*}},%{{.*}})
; amx-int8-evex
; CHECK:    tdpbssde %tmm7, %tmm4, %tmm3
; CHECK:    tdpbsude %tmm7, %tmm4, %tmm3
; CHECK:    tdpbusde %tmm7, %tmm0, %tmm3
; CHECK:    tdpbuude %tmm1, %tmm4, %tmm3
; amx-tile-evex
; CHECK:    tileloadde (%{{.*}},%{{.*}}), %tmm3
; CHECK:    tileloaddt1e (%{{.*}},%{{.*}}), %tmm3
; CHECK:    tilemove %tmm7, %tmm4
; CHECK:    tilestorede %tmm3, (%{{.*}},%{{.*}})
; CHECK:    tilezeroe %tmm3
; amx-transpose
; CHECK:    t2rpntlvw       %{{.*}}, (%{{.*}},%{{.*}}), %tmm0
; CHECK:    t2rpntlvwt1     %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2transposew    %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2transposewt1  %{{.*}}, (%{{.*}},%{{.*}}), %tmm4


define void @test_amx(i64 %addr, i64 %addrx, i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1,i8* %addr2, <16 x float> %zmm, <4 x float> %xmm) {
; amx-avx512
call <16 x float> @llvm.x86.tcvtrowd2psei(i8 7, i32 9)
call <16 x float> @llvm.x86.tcvtrowd2psee(i8 7, i32 %rv32)
call void @llvm.x86.tile16move(i8 4, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm)
call <16 x float> @llvm.x86.tilemovei(i8 7, i8 9)
call <16 x float> @llvm.x86.tilemovee(i8 7, i32 %rv32)
call <16 x float> @llvm.x86.tilemovex(i8 7, <4 x float> %xmm)
; amx-bf16-evex
call void @llvm.x86.tdpbf16pse(i8 3, i8 4, i8 7)
; amx-element-evex
call void @llvm.x86.tcvtd2pse  (i8* %addr1, i64 %stride, i8 1)
; amx-int8-evex
call void @llvm.x86.tdpbssde(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbsude(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbusde(i8 3, i8 0, i8 7)
call void @llvm.x86.tdpbuude(i8 3, i8 4, i8 1)
; amx-tile-evex
call void @llvm.x86.tileloadde64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tileloaddt1e64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilemove(i8 4, i8 7)
call void @llvm.x86.tilestorede64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilezeroe(i8 3)
; amx-transpose
call void @llvm.x86.t2rpntlvw  (i8 1, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2rpntlvwt1   (i8 2, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2transposew  (i8 3, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2transposewt1(i8 4, i8* %addr1, i64 %stride, i64 %rvalue)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-avx512
declare void @llvm.x86.tile16move(i8 %tile0, <16 x float> %zmm0, <16 x float> %zmm1, <16 x float> %zmm2, <16 x float> %zmm3, <16 x float> %zmm4, <16 x float> %zmm5, <16 x float> %zmm6, <16 x float> %zmm7, <16 x float> %zmm8, <16 x float> %zmm9, <16 x float> %zmm10, <16 x float> %zmm11, <16 x float> %zmm12, <16 x float> %zmm13, <16 x float> %zmm14, <16 x float> %zmm15)
declare <16 x float> @llvm.x86.tilemovei(i8 %tile0, i8 %tile1)
declare <16 x float> @llvm.x86.tcvtrowd2psei(i8 %tile0, i32 %src1)
declare <16 x float> @llvm.x86.tcvtrowd2psee(i8 %tile0, i32 %rv32)
declare <16 x float> @llvm.x86.tilemovee(i8 %tile0, i32 %rv32)
declare <16 x float> @llvm.x86.tilemovex(i8 %tile0, <4 x float> %xmm)
; amx-bf16-evex
declare void @llvm.x86.tdpbf16pse(i8 %tile0, i8 %tile1, i8 %tile2)
; amx-element-evex
declare void @llvm.x86.tcvtd2pse  (i8* %addr1, i64 %stride, i8 %tile1)
; amx-int8-evex
declare void @llvm.x86.tdpbssde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbsude(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbusde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbuude(i8 %tile0, i8 %tile1, i8 %tile2)
; amx-tile-evex
declare void @llvm.x86.tileloadde64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tileloaddt1e64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilemove(i8 %tile0, i8 %tile1)
declare void @llvm.x86.tilestorede64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilezeroe(i8 %tile)
; amx-transpose
declare void @llvm.x86.t2rpntlvw     (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2rpntlvwt1   (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2transposew  (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2transposewt1(i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
