; REVIEW: Add divide once it is implemented for 64-bit.
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s

target datalayout = "e-p:64:64"

@g = common global <8 x double> zeroinitializer, align 64

define <8 x double> @add1(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vaddpd g(%rip), %zmm0, %zmm0
  %tmp1 = load <8 x double>* @g, align 64
  %add = fadd <8 x double> %tmp1, %a
  ret <8 x double> %add
}

define <8 x double> @add2(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vaddpd g(%rip), %zmm0, %zmm0
  %tmp = load <8 x double>* @g, align 64
  %add = fadd <8 x double> %a, %tmp
  ret <8 x double> %add
}

define <8 x double> @mul1(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vmulpd g(%rip), %zmm0, %zmm0
  %tmp1 = load <8 x double>* @g, align 64
  %mul = fmul <8 x double> %tmp1, %a
  ret <8 x double> %mul
}

define <8 x double> @mul2(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vmulpd g(%rip), %zmm0, %zmm0
  %tmp = load <8 x double>* @g, align 64
  %mul = fmul <8 x double> %a, %tmp
  ret <8 x double> %mul
}

define <8 x double> @sub1(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vsubpd g(%rip), %zmm0, %zmm0
  %tmp1 = load <8 x double>* @g, align 64
  %sub = fsub <8 x double> %a, %tmp1
  ret <8 x double> %sub
}

define <8 x double> @sub2(<8 x double> %a) nounwind readonly ssp {
entry:
; CHECK:  vsubrpd g(%rip), %zmm0, %zmm0
  %tmp = load <8 x double>* @g, align 64
  %sub = fsub <8 x double> %tmp, %a
  ret <8 x double> %sub
}
