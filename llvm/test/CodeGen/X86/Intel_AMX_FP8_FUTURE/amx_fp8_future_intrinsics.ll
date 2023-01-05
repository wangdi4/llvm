; REQUIRES: intel_feature_isa_amx_fp8_future
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+amx-tile,+amx-fp8 | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; CHECK:    ttdpbf8ps        %tmm3, %tmm2, %tmm1
; CHECK:    ttdpbhf8ps        %tmm3, %tmm2, %tmm1
; CHECK:    ttdphbf8ps        %tmm3, %tmm2, %tmm1
; CHECK:    ttdphf8ps        %tmm3, %tmm2, %tmm1

define void @test_amx(){
call void @llvm.x86.ttdpbf8ps(i8 1, i8 2, i8 3)
call void @llvm.x86.ttdpbhf8ps(i8 1, i8 2, i8 3)
call void @llvm.x86.ttdphbf8ps(i8 1, i8 2, i8 3)
call void @llvm.x86.ttdphf8ps(i8 1, i8 2, i8 3)
ret void
}
declare void @llvm.x86.ttdpbf8ps(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.ttdpbhf8ps(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.ttdphbf8ps(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.ttdphf8ps(i8 %tile0, i8 %tile1, i8 %tile2)
