; REQUIRES: intel_feature_isa_amx_fp16
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-bf16,+amx-int8,+amx-fp16 | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-fp16
; CHECK:    tdpfp16ps       %tmm1, %tmm2, %tmm3

define void @test_amx() {
; amx-avx512
; amx-fp16
call void @llvm.x86.tdpfp16ps(i8 3, i8 2, i8 1)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-fp16
declare void @llvm.x86.tdpfp16ps(i8 %tile3, i8 %tile2, i8 %tile1)
