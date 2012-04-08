; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define <8 x i64> @load1(<8 x i64>* %p) nounwind {
entry:
; KNF: vloadunpack{{[hl]}}q {{(64)?\(%[a-z]+\)}}, [[R1:%v[0-9]+]]
; KNF: vloadunpack{{[hl]}}q {{(64)?\(%[a-z]+\)}}, [[R1]]
  %0 = load <8 x i64>* %p, align 8
  ret <8 x i64> %0
}

define <8 x i64> @load2(<8 x i64>* %p) nounwind {
entry:
; KNF: vloadunpack{{[hl]}}q {{(192|256)\(%[a-z]+\)}}, [[R1:%v[0-9]+]]
; KNF: vloadunpack{{[hl]}}q {{(192|256)\(%[a-z]+\)}}, [[R1]]
  %ptr = getelementptr <8 x i64>* %p, i32 3
  %0 = load <8 x i64>* %ptr, align 8
  ret <8 x i64> %0
}

define void @store1(<8 x i64>* %p, <8 x i64> %v) nounwind {
entry:
; KNF: vpackstore{{[hl]}}q [[R1:%v[0-9]+]], {{(64)?\(%[a-z]+\)}}
; KNF: vpackstore{{[hl]}}q [[R1]], {{(64)?\(%[a-z]+\)}}
  store <8 x i64> %v, <8 x i64>* %p, align 8
  ret void
}

define void @store2(<8 x i64>* %p, <8 x i64> %v) nounwind {
entry:
; KNF: vpackstore{{[hl]}}q [[R1:%v[0-9]+]], {{(192|256)\(%[a-z]+\)}}
; KNF: vpackstore{{[hl]}}q [[R1]], {{(192|256)\(%[a-z]+\)}}
  %ptr = getelementptr <8 x i64>* %p, i32 3
  store <8 x i64> %v, <8 x i64>* %ptr, align 8
  ret void
}


