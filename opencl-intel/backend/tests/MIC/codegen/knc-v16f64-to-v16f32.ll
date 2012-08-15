; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;


target datalayout = "e-p:64:64"

define <16 x float> @d2f_trunc_vec(<16 x double> %v1) nounwind {
entry:
; KNC: vcvtpd2ps %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: movl      $65280, {{%[a-z0-9]+}}
; KNC: vcvtpd2ps %zmm{{[0-9]+}}, %zmm{{[0-9]+}}
; KNC: kmov      {{%[a-z0-9]+}}, [[K1:%k[0-7]+]]
; KNC: vpermf32x4 $64, %zmm{{[0-9]+}}, %zmm{{[0-9]+}}{[[K1]]}

  %f1 = fptrunc <16 x double> %v1 to <16 x float>
  ret <16 x float> %f1
}
