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

define <8 x i64> @and1(<8 x i64> %a, <8 x i64> %b) nounwind readnone ssp {
entry:
; KNC: and1
; KNC: vpandq {{%zmm[0-9]+}}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
; KNC: ret
  %and = and <8 x i64> %a, %b
  ret <8 x i64> %and
}

define <8 x i64> @and2(<8 x i64>* nocapture %a, <8 x i64> %b) nounwind readonly ssp {
entry:
; KNC: and2
; KNC: vpandq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x i64>* %a, align 64
  %and = and <8 x i64> %tmp1, %b
  ret <8 x i64> %and
}

define <8 x i64> @and3(<8 x i64> %a, <8 x i64>* nocapture %b) nounwind readonly ssp {
entry:
; KNC: and3
; KNC: vpandq {{\(%[a-z]+\)}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp2 = load <8 x i64>* %b, align 64
  %and = and <8 x i64> %tmp2, %a
  ret <8 x i64> %and
}

define <8 x i64> @and4(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNC: and4
; KNC: vpandq {{[^(]+\(%rip\)}}, {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x i64>* @gb, align 64
  %and = and <8 x i64> %tmp1, %a
  ret <8 x i64> %and
}

define <8 x i64> @and5(<8 x i64> %a) nounwind readonly ssp {
entry:
; KNC: and5
; KNC: movq {{[^(]+\(%rip\)}}, [[R1:%[a-z]+]]
; KNC: vpandq ([[R1]]), {{%zmm[0-9]+}}
; KNC: ret
  %tmp1 = load <8 x i64>** @pgb, align 8
  %tmp2 = load <8 x i64>* %tmp1, align 64
  %and = and <8 x i64> %tmp2, %a
  ret <8 x i64> %and
}


