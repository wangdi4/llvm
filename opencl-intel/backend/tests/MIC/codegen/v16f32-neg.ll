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

@g = common global <16 x float> zeroinitializer, align 64
@pg = common global <16 x float>* null, align 8

define <16 x float> @negate1(<16 x float> %a) nounwind readnone ssp {
entry:
; KNF: vxorpi {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <16 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %a
  ret <16 x float> %sub
}

define <16 x float> @negate2() nounwind readonly ssp {
entry:
  %tmp = load <16 x float>* @g, align 64
; KNF: vxorpi {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <16 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %tmp
  ret <16 x float> %sub
}

define <16 x float> @negate3() nounwind readonly ssp {
entry:
  %tmp = load <16 x float>** @pg, align 8
  %tmp1 = load <16 x float>* %tmp, align 64
; KNF: vxorpi {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxord {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <16 x float> <float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00, float -0.000000e+00>, %tmp1
  ret <16 x float> %sub
}
