; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@f = common global float 0.000000e+00, align 4    ; <float*> [#uses=1]

define float @load() nounwind readonly ssp {
entry:
; KNF: vloadd {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}
;
; KNC: vbroadcastss {{[^(]+}}(%rip), {{%zmm[0-9]+}}
  %tmp = load float* @f                           ; <float> [#uses=1]
  ret float %tmp
}
