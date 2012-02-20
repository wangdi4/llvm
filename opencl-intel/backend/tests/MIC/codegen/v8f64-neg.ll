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

@g = common global <8 x double> zeroinitializer, align 64
@pg = common global <8 x double>* null, align 8

define <8 x double> @negate1(<8 x double> %a) nounwind readnone ssp {
entry:
; KNF: vxorpq {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <8 x double> <double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00>, %a
  ret <8 x double> %sub
}

define <8 x double> @negate2() nounwind readonly ssp {
entry:
  %tmp = load <8 x double>* @g, align 64
; KNF: vxorpq {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <8 x double> <double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00>, %tmp
  ret <8 x double> %sub
}

define <8 x double> @negate3() nounwind readonly ssp {
entry:
  %tmp = load <8 x double>** @pg, align 8
  %tmp1 = load <8 x double>* %tmp, align 64
; KNF: vxorpq {{[^(]+}}(%rip), {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip), {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub <8 x double> <double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00, double -0.000000e+00>, %tmp1
  ret <8 x double> %sub
}
