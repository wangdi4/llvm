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

@g = common global double zeroinitializer, align 4
@pg = common global double* null, align 8

define double @negate1(double %a) nounwind readnone ssp {
entry:
; KNF: vxorpq {{[^(]+}}(%rip){1to8}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip){1to8}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub double -0.000000e+00, %a
  ret double %sub
}

define double @negate2() nounwind readonly ssp {
entry:
  %tmp = load double* @g, align 4
; KNF: vxorpq {{[^(]+}}(%rip){1to8}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip){1to8}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub double -0.000000e+00, %tmp
  ret double %sub
}

define double @negate3() nounwind readonly ssp {
entry:
  %tmp = load double** @pg, align 8
  %tmp1 = load double* %tmp, align 4
; KNF: vxorpq {{[^(]+}}(%rip){1to8}, {{%v[0-9]+}}, {{%v[0-9]+}}
;
; KNC: vpxorq {{[^(]+}}(%rip){1to8}, {{%zmm[0-9]+}}, {{%zmm[0-9]+}}
  %sub = fsub double -0.000000e+00, %tmp1
  ret double %sub
}
