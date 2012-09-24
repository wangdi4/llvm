; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN: | FileCheck %s

; Shuffle128x32 Intrinsic

;declare <16 x float> @llvm.x86.mic.mask.shuf128x32(<16 x float>, i16, <16 x float>, i32, i32) nounwind readnone

;define <16 x float> @test_1(<16 x float> %arg1, <16 x float> %arg2) {
; XX: vpermf32x4
; XX: vpshufd
;  %res = call <16 x float> @llvm.x86.mic.mask.shuf128x32(<16 x float> %arg1, i16 2570, <16 x float> %arg2, i32 250, i32 114) nounwind
;  ret <16 x float> %res
;}

;declare <16 x float> @llvm.x86.mic.shuf128x32(<16 x float>, i32, i32) nounwind readnone

;define <16 x float> @test_2(<16 x float> %arg1, <16 x float> %arg2) {
; XX: vpermf32x4
; XX: vpshufd
;  %res = call <16 x float> @llvm.x86.mic.shuf128x32(<16 x float> %arg1, i32 250, i32 114) nounwind
;  ret <16 x float> %res
;}

; Perm Intrinsic

declare <16 x float> @llvm.x86.mic.permute4f128(<16 x float>, i32) nounwind readnone

define <16 x float> @test_3(<16 x float> %arg1, <16 x float> %arg2) {
; CHECK: vpermf32x4
  %res = call <16 x float> @llvm.x86.mic.permute4f128(<16 x float> %arg1, i32 250) nounwind
  ret <16 x float> %res
}

declare <16 x float> @llvm.x86.mic.mask.permute4f128(<16 x float>, i16, <16 x float>, i32) nounwind readnone

define <16 x float> @test_4(<16 x float> %arg1, <16 x float> %arg2) {
; CHECK: vpermf32x4
  %res = call <16 x float> @llvm.x86.mic.mask.permute4f128(<16 x float> %arg1, i16 2570, <16 x float> %arg2, i32 250) nounwind
  ret <16 x float> %res
}

; Shuffle Intrinsic

declare <16 x float> @llvm.x86.mic.shuffle(<16 x float>, i32) nounwind readnone

define <16 x float> @test_5(<16 x float> %arg1, <16 x float> %arg2) {
; CHECK: vpshufd
  %res = call <16 x float> @llvm.x86.mic.shuffle(<16 x float> %arg1, i32 250) nounwind
  ret <16 x float> %res
}

declare <16 x float> @llvm.x86.mic.mask.shuffle(<16 x float>, i16, <16 x float>, i32) nounwind readnone

define <16 x float> @test_6(<16 x float> %arg1, <16 x float> %arg2) {
; CHECK: vpshufd
  %res = call <16 x float> @llvm.x86.mic.mask.shuffle(<16 x float> %arg1, i16 2570, <16 x float> %arg2, i32 250) nounwind
  ret <16 x float> %res
}
