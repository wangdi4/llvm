; REQUIRES: intel_feature_isa_amx2
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-element,+amx-format,+amx-fp16,+amx-bf16,+amx-int8,+amx-memory,+amx-reduce,+amx-transpose,+amx-avx512,+amx-tile-evex,+amx-int8-evex,+amx-bf16-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    taddps  %tmm1, %tmm2, %tmm3
; CHECK:    tandd   %tmm6, %tmm5, %tmm4
; CHECK:    tandnd  %tmm7, %tmm2, %tmm6
; CHECK:    tcmpps  $4, %tmm2, %tmm3, %tmm7
; CHECK:    tcvtb2ps        %tmm1, %tmm2
; CHECK:    tcvtbf162ps     %tmm4, %tmm3
; CHECK:    tcvtd2ps        %tmm5, %tmm6
; CHECK:    tcvtps2bf16     %tmm7, %tmm3
; CHECK:    tcvtps2bs       %tmm1, %tmm2
; CHECK:    tcvtps2ubs      %tmm6, %tmm2
; CHECK:    tcvtub2ps       %tmm7, %tmm2
; CHECK:    tfmaddps        %tmm7, %tmm2, %tmm3
; CHECK:    tfmsubps        %tmm6, %tmm2, %tmm3
; CHECK:    tfnmaddps       %tmm5, %tmm2, %tmm3
; CHECK:    tfnmsubps %tmm1, %tmm2, %tmm3
; CHECK:    tmaxps %tmm1, %tmm4, %tmm3
; CHECK:    tminps %tmm1, %tmm5, %tmm3
; CHECK:    tmulps %tmm1, %tmm6, %tmm3
; CHECK:    tord   %tmm1, %tmm7, %tmm3
; CHECK:    trcp14ps        %tmm6, %tmm2
; CHECK:    treduceps       $6, %tmm2, %tmm7
; CHECK:    tscalefps       %tmm1, %tmm2, %tmm3
; CHECK:    tslld   $4, %tmm2, %tmm7
; CHECK:    tsrld   $3, %tmm2, %tmm6
; CHECK:    tsrlvd  $1, %tmm2, %tmm3
; CHECK:    tsubps  %tmm1, %tmm4, %tmm3
; CHECK:    txord   %tmm5, %tmm2, %tmm3
; CHECK:    tblendvd        %tmm1, %tmm2, %tmm7
; CHECK:    tinterleaveeb   %tmm1, %tmm2, %tmm3
; CHECK:    tinterleaveew   %tmm1, %tmm2, %tmm4
; CHECK:    tinterleaveob   %tmm1, %tmm5, %tmm3
; CHECK:    tinterleaveow   %tmm6, %tmm2, %tmm3
; CHECK:    tnarrowb        $7, %tmm2, %tmm6
; CHECK:    tnarroww        $5, %tmm2, %tmm7
; CHECK:    tpermb  %tmm4, %tmm2, %tmm3
; CHECK:    tpermd  %tmm6, %tmm2, %tmm3
; CHECK:    tpermw  %tmm7, %tmm2, %tmm3
; CHECK:    twidenb $1, %tmm2, %tmm7
; CHECK:    twidenw $3, %tmm2, %tmm4
; CHECK:    tdpfp16ps       %tmm1, %tmm2, %tmm3
; CHECK:    tgatherrowd     (%{{.*}},%{{.*}}), %tmm1
; CHECK:    tgatherrowdt1   (%{{.*}},%{{.*}}), %tmm2
; CHECK:    tgatherrowq     (%{{.*}},%{{.*}}), %tmm3
; CHECK:    tgatherrowqt1   (%{{.*}},%{{.*}}), %tmm4
; CHECK:    tscatterrowd    %tmm5, (%{{.*}},%{{.*}})
; CHECK:    tscatterrowdt1  %tmm6, (%{{.*}},%{{.*}})
; CHECK:    tscatterrowq    %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tscatterrowqt1  %tmm7, (%{{.*}},%{{.*}})
; CHECK:    tstorehd        %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstorehdt1      %tmm3, (%{{.*}},%{{.*}})
; CHECK:    tstorentd       %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstoreqd        %tmm5, (%{{.*}},%{{.*}})
; CHECK:    tstoreqdt1      %tmm1, (%{{.*}},%{{.*}})
; CHECK:    tstorerowd      %tmm1, (%{{.*}})
; CHECK:    tcoladdbcastps  %tmm1, %tmm2
; CHECK:    tcoladdps       %tmm1, (%{{.*}})
; CHECK:    t2rpntlvw       %{{.*}}, (%{{.*}},%{{.*}}), %tmm0
; CHECK:    t2rpntlvwt1     %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2transposew    %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2transposewt1  %{{.*}}, (%{{.*}},%{{.*}}), %tmm4
; CHECK:    taddps  (%{{.*}}), %tmm2, %tmm3
; CHECK:    tandd   (%{{.*}}), %tmm2, %tmm4
; CHECK:    tandnd  (%{{.*}}), %tmm5, %tmm3
; CHECK:    tmaxps  (%{{.*}}), %tmm2, %tmm1
; CHECK:    tminps  (%{{.*}}), %tmm2, %tmm3
; CHECK:    tmulps  (%{{.*}}), %tmm4, %tmm3
; CHECK:    tscalefps       (%{{.*}}), %tmm2, %tmm5
; CHECK:    tsrlvd  (%{{.*}}), %tmm6, %tmm3
; CHECK:    tsubps  (%{{.*}}), %tmm2, %tmm7
; CHECK:    txord   (%{{.*}}), %tmm1, %tmm3
; CHECK:    tord    (%{{.*}}), %tmm2, %tmm3
; CHECK:    tpermb  (%{{.*}}), %tmm2, %tmm5
; CHECK:    tpermd  (%{{.*}}), %tmm4, %tmm3
; CHECK:    tpermw  (%{{.*}}), %tmm2, %tmm3
; CHECK:    tfmaddps        (%{{.*}}), %tmm2, %tmm1
; CHECK:    tfmsubps        (%{{.*}}), %tmm4, %tmm3
; CHECK:    tfnmaddps       (%{{.*}}), %tmm2, %tmm3
; CHECK:    tfnmsubps       (%{{.*}}), %tmm2, %tmm7
; CHECK:    tcmpps  $1, (%{{.*}}), %tmm3, %tmm6
; CHECK:    tbroadcastrowd  (%{{.*}}), %tmm4
; CHECK:    tileloadde (%{{.*}},%{{.*}}), %TMM2_TMM3
; CHECK:    tileloaddt1e (%{{.*}},%{{.*}}), %TMM2_TMM3
; CHECK:    tilestorede %TMM2_TMM3, (%{{.*}},%{{.*}})
; CHECK:    tilezeroe %tmm3
; CHECK:    tdpbssde %tmm7, %tmm4, %tmm3
; CHECK:    tdpbsude %tmm7, %tmm4, %tmm3
; CHECK:    tdpbusde %tmm7, %tmm0, %tmm3
; CHECK:    tdpbuude %tmm1, %tmm4, %tmm3
; CHECK:    tdpbf16pse %tmm7, %tmm4, %tmm3
; CHECK:    tilemove %tmm7, %tmm4
; CHECK:    tilemovrowe $9, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrowe %{{.*}}, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrowe %xmm{{.*}}, %tmm7, %zmm{{.*}}
; CHECK:    tile16move %zmm{{.*}}, %tmm4

define void @test_amx(i64 %addr, i64 %addrx, i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1,i8* %addr2, <16 x float> %zmm, <4 x float> %xmm) {
call void @llvm.x86.taddps.reg(i8 3, i8 2, i8 1)
call void @llvm.x86.tandd.reg(i8 4, i8 5, i8 6)
call void @llvm.x86.tandnd.reg(i8 6, i8 2, i8 7)
call void @llvm.x86.tcmpps.reg(i8 7, i8 3, i8 2, i8 4)
call void @llvm.x86.tcvtb2ps(i8 2, i8 1)
call void @llvm.x86.tcvtbf162ps(i8 3, i8 4)
call void @llvm.x86.tcvtd2ps(i8 6, i8 5)
call void @llvm.x86.tcvtps2bf16(i8 3, i8 7)
call void @llvm.x86.tcvtps2bs(i8 2, i8 1)
call void @llvm.x86.tcvtps2ubs(i8 2, i8 6)
call void @llvm.x86.tcvtub2ps(i8 2, i8 7)
call void @llvm.x86.tfmaddps.reg(i8 3, i8 2, i8 7)
call void @llvm.x86.tfmsubps.reg(i8 3, i8 2, i8 6)
call void @llvm.x86.tfnmaddps.reg(i8 3, i8 2, i8 5)
call void @llvm.x86.tfnmsubps.reg(i8 3, i8 2, i8 1)
call void @llvm.x86.tmaxps.reg(i8 3, i8 4, i8 1)
call void @llvm.x86.tminps.reg(i8 3, i8 5, i8 1)
call void @llvm.x86.tmulps.reg(i8 3, i8 6, i8 1)
call void @llvm.x86.tord.reg(i8 3, i8 7, i8 1)
call void @llvm.x86.trcp14ps(i8 2, i8 6)
call void @llvm.x86.treduceps(i8 7, i8 2, i8 6)
call void @llvm.x86.tscalefps.reg(i8 3, i8 2, i8 1)
call void @llvm.x86.tslld(i8 7, i8 2, i8 4)
call void @llvm.x86.tsrld(i8 6, i8 2, i8 3)
call void @llvm.x86.tsrlvd.reg(i8 3, i8 2, i8 1)
call void @llvm.x86.tsubps.reg(i8 3, i8 4, i8 1)
call void @llvm.x86.txord.reg(i8 3, i8 2, i8 5)
call void @llvm.x86.tblendvd(i8 7, i8 2, i8 1)
call void @llvm.x86.tinterleaveeb(i8 3, i8 2, i8 1)
call void @llvm.x86.tinterleaveew(i8 4, i8 2, i8 1)
call void @llvm.x86.tinterleaveob(i8 3, i8 5, i8 1)
call void @llvm.x86.tinterleaveow(i8 3, i8 2, i8 6)
call void @llvm.x86.tnarrowb(i8 6, i8 2, i8 7)
call void @llvm.x86.tnarroww(i8 7, i8 2, i8 5)
call void @llvm.x86.tpermb.reg(i8 3, i8 2, i8 4)
call void @llvm.x86.tpermd.reg(i8 3, i8 2, i8 6)
call void @llvm.x86.tpermw.reg(i8 3, i8 2, i8 7)
call void @llvm.x86.twidenb(i8 7, i8 2, i8 1)
call void @llvm.x86.twidenw(i8 4, i8 2, i8 3)
call void @llvm.x86.tdpfp16ps(i8 3, i8 2, i8 1)
call void @llvm.x86.tgatherrowd(i8 1, i8* %addr1, i8* %addr2)
call void @llvm.x86.tgatherrowdt1(i8 2, i8* %addr1, i8* %addr2)
call void @llvm.x86.tgatherrowq(i8 3, i8* %addr1, i8* %addr2)
call void @llvm.x86.tgatherrowqt1(i8 4, i8* %addr1, i8* %addr2)
call void @llvm.x86.tscatterrowd  (i8* %addr1, i8* %addr2, i8 5)
call void @llvm.x86.tscatterrowdt1(i8* %addr1, i8* %addr2, i8 6)
call void @llvm.x86.tscatterrowq  (i8* %addr1, i8* %addr2, i8 1)
call void @llvm.x86.tscatterrowqt1(i8* %addr1, i8* %addr2, i8 7)
call void @llvm.x86.tstorehd  (i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstorehdt1(i8* %addr1, i64 %stride, i8 3)
call void @llvm.x86.tstorentd (i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstoreqd  (i8* %addr1, i64 %stride, i8 5)
call void @llvm.x86.tstoreqdt1(i8* %addr1, i64 %stride, i8 1)
call void @llvm.x86.tstorerowd(i8* %addr1, i8 7)
call void @llvm.x86.tcoladdbcastps(i8 2, i8 1)
call void @llvm.x86.tcoladdps(i8* %addr1, i8 3)
call void @llvm.x86.t2rpntlvw  (i8 1, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2rpntlvwt1   (i8 2, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2transposew  (i8 3, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2transposewt1(i8 4, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.taddps.mem(i8 3, i8 2, i8* %addr1)
call void @llvm.x86.tandd.mem(i8 4, i8 2, i8* %addr1)
call void @llvm.x86.tandnd.mem(i8 3, i8 5, i8* %addr1)
call void @llvm.x86.tmaxps.mem(i8 1, i8 2, i8* %addr1)
call void @llvm.x86.tminps.mem(i8 3, i8 2, i8* %addr1)
call void @llvm.x86.tmulps.mem(i8 3, i8 4, i8* %addr1)
call void @llvm.x86.tscalefps.mem(i8 5, i8 2, i8* %addr1)
call void @llvm.x86.tsrlvd.mem(i8 3, i8 6, i8* %addr1)
call void @llvm.x86.tsubps.mem(i8 7, i8 2, i8* %addr1)
call void @llvm.x86.txord.mem(i8 3, i8 1, i8* %addr1)
call void @llvm.x86.tord.mem(i8 3, i8 2, i8* %addr1)
call void @llvm.x86.tpermb.mem(i8 5, i8 2, i8* %addr1)
call void @llvm.x86.tpermd.mem(i8 3, i8 4, i8* %addr1)
call void @llvm.x86.tpermw.mem(i8 3, i8 2, i8* %addr1)
call void @llvm.x86.tfmaddps.mem(i8 1, i8 2, i8* %addr1)
call void @llvm.x86.tfmsubps.mem(i8 3, i8 4, i8* %addr1)
call void @llvm.x86.tfnmaddps.mem(i8 3, i8 2, i8* %addr1)
call void @llvm.x86.tfnmsubps.mem(i8 7, i8 2, i8* %addr1)
call void @llvm.x86.tcmpps.mem(i8 6, i8 3, i8* %addr1, i8 1)
call void @llvm.x86.tbroadcastrowd(i8 4, i8* %addr1)
call void @llvm.x86.tileloadde64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tileloaddt1e64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilestorede64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilezeroe(i8 3)
call void @llvm.x86.tdpbssde(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbsude(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbusde(i8 3, i8 0, i8 7)
call void @llvm.x86.tdpbuude(i8 3, i8 4, i8 1)
call void @llvm.x86.tdpbf16pse(i8 3, i8 4, i8 7)
call void @llvm.x86.tilemove(i8 4, i8 7)
call <16 x float> @llvm.x86.tilemovei(i8 7, i8 9)
call <16 x float> @llvm.x86.tilemovee(i8 7, i32 %rv32)
call <16 x float> @llvm.x86.tilemovex(i8 7, <4 x float> %xmm)
call void @llvm.x86.tile16move(i8 4, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm, <16 x float> %zmm)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
declare void @llvm.x86.tcmpps.mem(i8 %tile8, i8 %tile3, i8* %addr, i8 %tile1)
declare void @llvm.x86.tfmaddps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tfmsubps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tfnmaddps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tfnmsubps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tpermb.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tpermd.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tpermw.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tscalefps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tsrlvd.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tsubps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.txord.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tord.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.taddps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tandd.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tandnd.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tmaxps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tminps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.tmulps.mem(i8 %tile3, i8 %tile2, i8* %addr)
declare void @llvm.x86.taddps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tandd.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tandnd.reg(i8 %tile3, i8 %tile2, i8 %n)
declare void @llvm.x86.tcmpps.reg(i8 %tile8, i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtb2ps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtbf162ps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtd2ps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtps2bf16(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtps2bs(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtps2ubs(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcvtub2ps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tfmaddps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tfmsubps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tfnmaddps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tfnmsubps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tmaxps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tminps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tmulps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tord.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.trcp14ps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.treduceps(i8 %tile8, i8 %tile2, i8 %n)
declare void @llvm.x86.tscalefps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tslld(i8 %tile8, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tsrld(i8 %tile8, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tsrlvd.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tsubps.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.txord.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tblendvd(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tinterleaveeb(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tinterleaveew(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tinterleaveob(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tinterleaveow(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tnarrowb(i8 %tile8, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tnarroww(i8 %tile8, i8 %tile2, i8 %n)
declare void @llvm.x86.tpermb.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tpermd.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tpermw.reg(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.twidenb(i8 %tile8, i8 %tile2, i8 %n)
declare void @llvm.x86.twidenw(i8 %tile8, i8 %tile2, i8 %n)
declare void @llvm.x86.tdpfp16ps(i8 %tile3, i8 %tile2, i8 %tile1)
declare void @llvm.x86.tbroadcastrowd(i8 %tile1, i8* %addr)
declare void @llvm.x86.tgatherrowd  (i8 %tile1, i8* %addr1, i8* %addr2)
declare void @llvm.x86.tgatherrowdt1(i8 %tile1, i8* %addr1, i8* %addr2)
declare void @llvm.x86.tgatherrowq  (i8 %tile1, i8* %addr1, i8* %addr2)
declare void @llvm.x86.tgatherrowqt1(i8 %tile1, i8* %addr1, i8* %addr2)
declare void @llvm.x86.tscatterrowd  (i8* %addr1, i8* %addr2, i8 %tile1)
declare void @llvm.x86.tscatterrowdt1(i8* %addr1, i8* %addr2, i8 %tile1)
declare void @llvm.x86.tscatterrowq  (i8* %addr1, i8* %addr2, i8 %tile1)
declare void @llvm.x86.tscatterrowqt1(i8* %addr1, i8* %addr2, i8 %tile1)
declare void @llvm.x86.tstorehd  (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorehdt1(i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorentd (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstoreqd  (i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstoreqdt1(i8* %addr1, i64 %stride, i8 %tile1)
declare void @llvm.x86.tstorerowd(i8* %addr1, i8 %tile1)
declare void @llvm.x86.tcoladdbcastps(i8 %tile2, i8 %tile1)
declare void @llvm.x86.tcoladdps(i8* %addr1, i8 %tile1)
declare void @llvm.x86.t2rpntlvw     (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2rpntlvwt1   (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2transposew  (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2transposewt1(i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.tileloadde64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tileloaddt1e64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilestorede64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilezeroe(i8 %tile)
declare void @llvm.x86.tdpbssde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbsude(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbusde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbuude(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbf16pse(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tilemove(i8 %tile0, i8 %tile1)
declare <16 x float> @llvm.x86.tilemovei(i8 %tile0, i8 %tile1)
declare <16 x float> @llvm.x86.tilemovee(i8 %tile0, i32 %rv32)
declare <16 x float> @llvm.x86.tilemovex(i8 %tile0, <4 x float> %xmm)
declare void @llvm.x86.tile16move(i8 %tile0, <16 x float> %zmm0, <16 x float> %zmm1, <16 x float> %zmm2, <16 x float> %zmm3, <16 x float> %zmm4, <16 x float> %zmm5, <16 x float> %zmm6, <16 x float> %zmm7, <16 x float> %zmm8, <16 x float> %zmm9, <16 x float> %zmm10, <16 x float> %zmm11, <16 x float> %zmm12, <16 x float> %zmm13, <16 x float> %zmm14, <16 x float> %zmm15)
