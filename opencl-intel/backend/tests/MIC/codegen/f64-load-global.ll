; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

@d = common global double 0.000000e+00, align 8   ; <double*> [#uses=1]

define double @load() nounwind readonly ssp {
entry:

; KNF: vloadq {{[^(]+}}(%rip){1to8}, {{%v[0-9]+}}
;
; KNC: movq {{[^(]+}}(%rip), [[R1:%r[a-z]+]]
; KNC: vbroadcastsd ([[R1]]), {{%zmm[0-9]+}}
  %tmp = load double* @d                          ; <double> [#uses=1]
  ret double %tmp
}
