; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @mul1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNF: mul1:
; KNF: vmulhpu
; KNF: vmadd231pi
; KNF: vmadd231pi
; KNF: vmullpi
;
; KNC: mul1:
; KNC: vpmulhud
; KNC: vpmadd231d
; KNC: vpmadd231d
; KNC: vpmulld
  %mul = mul nsw <8 x i64> %a, %b
  ret <8 x i64> %mul
}

define <8 x i64> @mul2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNF: mul2:
; KNF: vmulhpu
; KNF: vmadd231pi
; KNF: vmadd231pi
; KNF: vmullpi
;
; KNC: mul2:
; KNC: vpmulhud
; KNC: vpmadd231d
; KNC: vpmadd231d
; KNC: vpmulld
  %tmp1 = load <8 x i64>* %a, align 64
  %mul = mul nsw <8 x i64> %tmp1, %b
  ret <8 x i64> %mul
}

define <8 x i64> @mul3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNF: mul3:
; KNF: vmulhpu
; KNF: vmadd231pi
; KNF: vmadd231pi
; KNF: vmullpi
;
; KNC: mul3:
; KNC: vpmulhud
; KNC: vpmadd231d
; KNC: vpmadd231d
; KNC: vpmulld

  %tmp2 = load <8 x i64>* %b, align 64
  %mul = mul nsw <8 x i64> %tmp2, %a
  ret <8 x i64> %mul
}

define <8 x i64> @mul4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: mul4:
; KNF: vmulhpu
; KNF: vmadd231pi
; KNF: vmadd231pi
; KNF: vmullpi
;
; KNC: mul4:
; KNC: vpmulhud
; KNC: vpmadd231d
; KNC: vpmadd231d
; KNC: vpmulld
%tmp1 = load <8 x i64>* @gb, align 64
  %mul = mul nsw <8 x i64> %tmp1, %a
  ret <8 x i64> %mul
}

define <8 x i64> @mul5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNF: mul5:
; KNF: vmulhpu
; KNF: vmadd231pi
; KNF: vmadd231pi
; KNF: vmullpi
;
; KNC: mul5:
; KNC: vpmulhud
; KNC: vpmadd231d
; KNC: vpmadd231d
; KNC: vpmulld
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %mul = mul nsw <8 x i64> %tmp2, %a
  ret <8 x i64> %mul
}
