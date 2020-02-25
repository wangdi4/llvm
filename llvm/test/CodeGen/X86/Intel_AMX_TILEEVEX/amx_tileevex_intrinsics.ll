; REQUIRES: intel_feature_isa_amx_tile_evex
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-tile-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-tile-evex
; CHECK:    tileloadde (%{{.*}},%{{.*}}), %tmm3
; CHECK:    tileloaddt1e (%{{.*}},%{{.*}}), %tmm3
; CHECK:    tilemove %tmm7, %tmm4
; CHECK:    tilestorede %tmm3, (%{{.*}},%{{.*}})
; CHECK:    tilezeroe %tmm3

define void @test_amx(i8* %addr1, i64 %stride) {
; amx-tile-evex
call void @llvm.x86.tileloadde64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tileloaddt1e64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilemove(i8 4, i8 7)
call void @llvm.x86.tilestorede64(i8 3, i8* %addr1, i64 %stride)
call void @llvm.x86.tilezeroe(i8 3)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-tile-evex
declare void @llvm.x86.tileloadde64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tileloaddt1e64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilemove(i8 %tile0, i8 %tile1)
declare void @llvm.x86.tilestorede64(i8 %tile, i8* %addr1, i64 %stride)
declare void @llvm.x86.tilezeroe(i8 %tile)
