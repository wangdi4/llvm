; REQUIRES: intel_feature_isa_amx_element_evex
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-element-evex | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-element-evex
; CHECK:    tcvtd2pse        %tmm1, (%{{.*}},%{{.*}})

define void @test_amx(i64 %stride, i8* %addr) {
; amx-element-evex
call void @llvm.x86.tcvtd2pse  (i8* %addr, i64 %stride, i8 1)
ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-element-evex
declare void @llvm.x86.tcvtd2pse  (i8* %addr, i64 %stride, i8 %tile1)
