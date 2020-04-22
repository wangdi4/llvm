; REQUIRES: intel_feature_isa_amx_int8_evex
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-int8-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-int8-evex
; CHECK:    tdpbssde %tmm7, %tmm4, %tmm3
; CHECK:    tdpbsude %tmm7, %tmm4, %tmm3
; CHECK:    tdpbusde %tmm7, %tmm0, %tmm3
; CHECK:    tdpbuude %tmm1, %tmm4, %tmm3

define void @test_amx() {
; amx-int8-evex
call void @llvm.x86.tdpbssde(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbsude(i8 3, i8 4, i8 7)
call void @llvm.x86.tdpbusde(i8 3, i8 0, i8 7)
call void @llvm.x86.tdpbuude(i8 3, i8 4, i8 1)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-int8-evex
declare void @llvm.x86.tdpbssde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbsude(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbusde(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.tdpbuude(i8 %tile0, i8 %tile1, i8 %tile2)
