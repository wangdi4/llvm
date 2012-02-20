; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; REVIEW: Both tests should generate FMA. Using FAM here is required for 
; correctness
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %s -check-prefix=KNFmpa 
;

target datalayout = "e-p:64:64"

@f = common global float 0.000000e+00, align 4    ; <float*> [#uses=8]

define float @add1(float %a) nounwind readonly ssp {
entry:
; KNF: vaddps  {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load float* @f                          ; <float> [#uses=1]
  %add = fadd float %tmp1, %a                     ; <float> [#uses=1]
  ret float %add
}

define float @add2(float %a) nounwind readonly ssp {
entry:
; KNF: vaddps  {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp = load float* @f                           ; <float> [#uses=1]
  %add = fadd float %a, %tmp                      ; <float> [#uses=1]
  ret float %add
}

define float @mul1(float %a) nounwind readonly ssp {
entry:
; KNF: vkmov {{%eax}}, {{%k[0-9]+}}
; KNF: vmulps {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load float* @f                          ; <float> [#uses=1]
  %mul = fmul float %tmp1, %a                     ; <float> [#uses=1]
  ret float %mul
}

define float @mul2(float %a) nounwind readonly ssp {
entry:
; KNF: vkmov {{%eax}}, {{%k[0-9]+}}
; KNF: vmulps {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp = load float* @f                           ; <float> [#uses=1]
  %mul = fmul float %a, %tmp                      ; <float> [#uses=1]
  ret float %mul
}

define float @sub1(float %a) nounwind readonly ssp {
entry:
; KNF: vkmov {{%eax}}, {{%k[0-9]+}}
; KNF: vsubps {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp1 = load float* @f                          ; <float> [#uses=1]
  %sub = fsub float %a, %tmp1                     ; <float> [#uses=1]
  ret float %sub
}

define float @sub2(float %a) nounwind readonly ssp {
entry:
; KNF: vkmov {{%eax}}, {{%k[0-9]+}}
; KNF: vsubrps {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp = load float* @f                           ; <float> [#uses=1]
  %sub = fsub float %tmp, %a                      ; <float> [#uses=1]
  ret float %sub
}

define float @div1(float %a) nounwind readonly ssp {
entry:
; KNF: vloadd {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}
; KNF: vkmov {{%eax}}, {{%k[0-9]+}}
; KNF: vrcpresps {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vrcprefineps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmsubr23c1ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmadd231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vfixupps
; KNF: vmsubr231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF:  vmadd132ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vfixupps

; KNFmpa: vloadd {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}
; KNFmpa: vkmov {{%eax}}, {{%k[0-9]+}}
; KNFmpa: vrcpresps {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vrcprefineps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmsubr23c1ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmadd231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vfixupps
; KNFmpa: vmsubr231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa:  vmadd132ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vfixupps
  %tmp1 = load float* @f                          ; <float> [#uses=1]
  %div = fdiv float %a, %tmp1                     ; <float> [#uses=1]
  ret float %div
}

define float @div2(float %a) nounwind readonly ssp {
entry:
; KNF: vrcpresps {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vrcprefineps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmadd231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmsubr231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vmadd132ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNF: vfixupps $132737, {{%v[0-9]+}}, {{%v[0-9]+}}


; KNFmpa: vrcpresps {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vrcprefineps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmadd231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmulps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmsubr231ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vmadd132ps {{%v[0-9]+}}, {{%v[0-9]+}}, {{%v[0-9]+}}
; KNFmpa: vfixupps $132737, {{%v[0-9]+}}, {{%v[0-9]+}}
  %tmp = load float* @f                           ; <float> [#uses=1]
  %div = fdiv float %tmp, %a                      ; <float> [#uses=1]
  ret float %div
}
