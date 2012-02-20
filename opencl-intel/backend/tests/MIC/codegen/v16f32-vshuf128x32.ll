; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.shuf128x32(<16 x float>, i32, i32)

define <16 x float> @shufreg(float %val) {
entry:
; KNF: vshuf128x32 $0, $0, {{%v[0-9]+}}, {{%v[0-9]+}}
  %temp255 = insertelement <16 x float> undef, float %val, i32 0
  %vec = call <16 x float> @llvm.x86.mic.shuf128x32(<16 x float> %temp255, i32 0, i32 0)
  
  ret <16 x float> %vec
}

define <16 x float> @shufmem(<16 x float>* %ptr) {
entry:
; KNF: vshuf128x32 $0, $0, ({{%[a-z]+}}), {{%v[0-9]+}}
  %temp255 = load <16 x float>* %ptr
  %vec = call <16 x float> @llvm.x86.mic.shuf128x32(<16 x float> %temp255, i32 0, i32 0)
  
  ret <16 x float> %vec
}

define <16 x float> @shufmemf32(float* %ptr) {
entry:
; KNF: vloadd ({{%[a-z]+}}){1to16}, [[R1:%v[0-9]+]]
; KNF: vshuf128x32 $0, $0, [[R1]], {{%v[0-9]+}}
  %val = load float* %ptr
  %temp255 = insertelement <16 x float> undef, float %val, i32 0
  %vec = call <16 x float> @llvm.x86.mic.shuf128x32(<16 x float> %temp255, i32 0, i32 0)
  
  ret <16 x float> %vec
}
