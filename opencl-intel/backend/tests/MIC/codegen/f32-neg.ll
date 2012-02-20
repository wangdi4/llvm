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

@g = common global float zeroinitializer, align 4
@pg = common global float* null, align 8

define float @negate1(float %a) nounwind readnone ssp {
entry:
; KNF: vxorpi {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip){1to16}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub float -0.000000e+00, %a
  ret float %sub
}

define float @negate2() nounwind readonly ssp {
entry:
  %tmp = load float* @g, align 4
; KNF: vxorpi {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip){1to16}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub float -0.000000e+00, %tmp
  ret float %sub
}

define float @negate3() nounwind readonly ssp {
entry:
  %tmp = load float** @pg, align 8
  %tmp1 = load float* %tmp, align 4
; KNF: vxorpi {{[^(]+}}(%rip){1to16}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip){1to16}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub float -0.000000e+00, %tmp1
  ret float %sub
}
