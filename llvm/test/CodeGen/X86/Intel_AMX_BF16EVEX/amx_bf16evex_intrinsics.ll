; REQUIRES: intel_feature_isa_amx_bf16_evex
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-bf16-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-bf16-evex
; CHECK:    tdpbf16pse %tmm7, %tmm4, %tmm3

define void @test_amx() {
; amx-bf16-evex
call void @llvm.x86.tdpbf16pse(i8 3, i8 4, i8 7)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-bf16-evex
declare void @llvm.x86.tdpbf16pse(i8 %tile0, i8 %tile1, i8 %tile2)
