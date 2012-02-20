; XFAIL: win32
; XFAIL: *
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

@gb = common global <16 x i64> zeroinitializer, align 128
@pgb = common global <16 x i64>* null, align 8

define <16 x i64> @shiftright1(<16 x i64> %a, <16 x i64> %b) nounwind readnone ssp {
entry:
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %shr = lshr <16 x i64> %a, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright2(<16 x i64>* nocapture %a, <16 x i64> %b) nounwind readonly ssp {
entry:
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %tmp1 = load <16 x i64>* %a
  %shr = lshr <16 x i64> %tmp1, %b
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright3(<16 x i64> %a, <16 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %tmp2 = load <16 x i64>* %b
  %shr = lshr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright4(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %tmp1 = load <16 x i64>* @gb, align 128
  %shr = lshr <16 x i64> %a, %tmp1
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright5(<16 x i64> %a) nounwind readonly ssp {
entry:
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %tmp1 = load <16 x i64>** @pgb
  %tmp2 = load <16 x i64>* %tmp1
  %shr = lshr <16 x i64> %a, %tmp2
  ret <16 x i64> %shr
}

define <16 x i64> @shiftright6(<16 x i64> %a) nounwind readnone ssp {
entry:
; Only 15 shrq's since the shift by 0 is optimized away
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
; CHECK: shrq
  %shr = lshr <16 x i64> %a, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  ret <16 x i64> %shr
}
