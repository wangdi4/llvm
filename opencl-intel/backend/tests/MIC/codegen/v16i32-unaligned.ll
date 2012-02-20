; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <16 x i32> @load1(<16 x i32>* %p) nounwind {
entry:
; KNF: vloadunpack{{[hl]}}d {{(64)?\(%[a-z]+\)}}, [[R1:%v[0-9]+]]
; KNF: vloadunpack{{[hl]}}d {{(64)?\(%[a-z]+\)}}, [[R1]]
  %0 = load <16 x i32>* %p, align 4
  ret <16 x i32> %0
}

define <16 x i32> @load2(<16 x i32>* %p) nounwind {
entry:
; KNF: vloadunpack{{[hl]}}d {{(192|256)\(%[a-z]+\)}}, [[R1:%v[0-9]+]]
; KNF: vloadunpack{{[hl]}}d {{(192|256)\(%[a-z]+\)}}, [[R1]]
  %ptr = getelementptr <16 x i32>* %p, i32 3
  %0 = load <16 x i32>* %ptr, align 4
  ret <16 x i32> %0
}

define void @store1(<16 x i32>* %p, <16 x i32> %v) nounwind {
entry:
; KNF: vpackstore{{[hl]}}d [[R1:%v[0-9]+]], {{(64)?\(%[a-z]+\)}}
; KNF: vpackstore{{[hl]}}d [[R1]], {{(64)?\(%[a-z]+\)}}
  store <16 x i32> %v, <16 x i32>* %p, align 4
  ret void
}

define void @store2(<16 x i32>* %p, <16 x i32> %v) nounwind {
entry:
; KNF: vpackstore{{[hl]}}d [[R1:%v[0-9]+]], {{(192|256)\(%[a-z]+\)}}
; KNF: vpackstore{{[hl]}}d [[R1]], {{(192|256)\(%[a-z]+\)}}
  %ptr = getelementptr <16 x i32>* %p, i32 3
  store <16 x i32> %v, <16 x i32>* %ptr, align 4
  ret void
}


