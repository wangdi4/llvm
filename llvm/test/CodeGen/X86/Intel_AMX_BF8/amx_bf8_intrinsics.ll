; REQUIRES: intel_feature_isa_amx_bf8
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-bf8 | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    tdpbf8ps        %tmm3, %tmm2, %tmm1

define void @test_amx(){
call void @llvm.x86.tdpbf8ps(i8 1, i8 2, i8 3)
ret void
}
declare void @llvm.x86.tdpbf8ps(i8 %tile0, i8 %tile1, i8 %tile2)
