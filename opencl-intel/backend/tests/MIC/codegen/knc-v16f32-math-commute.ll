; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
; REVIEW: Both tests should generate FMA. Using FAM here is required for 
; correctness
;
; REVIEW: Disabled KNC for now because we don't have divide.
;

target datalayout = "e-p:64:64"

@g = common global <16 x float> zeroinitializer, align 64

define <16 x float> @add1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: add1:
; KNC: vaddps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %add = fadd <16 x float> %tmp1, %a
  ret <16 x float> %add
}

define <16 x float> @add2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: add2:
; KNC: vaddps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %add = fadd <16 x float> %a, %tmp
  ret <16 x float> %add
}

define <16 x float> @mul1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: mul1:
; KNC: vmulps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %mul = fmul <16 x float> %tmp1, %a
  ret <16 x float> %mul
}

define <16 x float> @mul2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: mul2:
; KNC: vmulps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %mul = fmul <16 x float> %a, %tmp
  ret <16 x float> %mul
}

define <16 x float> @sub1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: sub1:
; KNC: vsubps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp1 = load <16 x float>* @g, align 64
  %sub = fsub <16 x float> %a, %tmp1
  ret <16 x float> %sub
}

define <16 x float> @sub2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: sub2:
; KNC: vsubrps g(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %tmp = load <16 x float>* @g, align 64
  %sub = fsub <16 x float> %tmp, %a
  ret <16 x float> %sub
}

define <16 x float> @div1(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: div1:
; KNC: vfixupnanps 
  %tmp1 = load <16 x float>* @g, align 64
  %div = fdiv <16 x float> %a, %tmp1
  ret <16 x float> %div
}

define <16 x float> @div2(<16 x float> %a) nounwind readonly ssp {
entry:
; KNC: div2:
; KNC: vfixupnanps 
  %tmp = load <16 x float>* @g, align 64
  %div = fdiv <16 x float> %tmp, %a
  ret <16 x float> %div
}
