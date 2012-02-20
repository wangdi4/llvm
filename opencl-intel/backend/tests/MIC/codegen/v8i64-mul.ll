; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

target datalayout = "e-p:64:64"

@gb = common global <8 x i64> zeroinitializer, align 64
@pgb = common global <8 x i64>* null, align 8

define <8 x i64> @mul1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: vmulhpu
; CHECK: vmullpi
; CHECK: vmadd231pi
; CHECK: vmadd231pi
  %mul = mul nsw <8 x i64> %a, %b
  ret <8 x i64> %mul
}

define <8 x i64> @mul2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: vmulhpu
; CHECK: vmullpi
; CHECK: vmadd231pi
; CHECK: vmadd231pi
  %tmp1 = load <8 x i64>* %a, align 64
  %mul = mul nsw <8 x i64> %tmp1, %b
  ret <8 x i64> %mul
}

define <8 x i64> @mul3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: vmulhpu
; CHECK: vmullpi
; CHECK: vmadd231pi
; CHECK: vmadd231pi
  %tmp2 = load <8 x i64>* %b, align 64
  %mul = mul nsw <8 x i64> %tmp2, %a
  ret <8 x i64> %mul
}

define <8 x i64> @mul4(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: vmulhpu
; CHECK: vmullpi
; CHECK: vmadd231pi
; CHECK: vmadd231pi
  %tmp1 = load <8 x i64>* @gb, align 64
  %mul = mul nsw <8 x i64> %tmp1, %a
  ret <8 x i64> %mul
}

define <8 x i64> @mul5(<8 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: vmulhpu
; CHECK: vmullpi
; CHECK: vmadd231pi
; CHECK: vmadd231pi
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %mul = mul nsw <8 x i64> %tmp2, %a
  ret <8 x i64> %mul
}
