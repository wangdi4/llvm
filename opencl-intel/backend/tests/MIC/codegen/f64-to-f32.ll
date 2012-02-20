; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define void @d2f_trunc_store(double %v1, float *%pf1) nounwind {
entry:
; KNF: vcvtpd2ps  $0, {rn}, %v0, [[R1:%v[0-9]+]]{%k1}
; KNF: vstored [[R1]]{a}, (%rdi)
  %f1 = fptrunc double %v1 to float
  store float %f1, float * %pf1, align 4
  ret void 
}

define float @d2f_trunc(double %v1) nounwind {
entry:
; KNF: vcvtpd2ps $0, {rn}, %v0, %v0{%k1}
  %f1 = fptrunc double %v1 to float
  ret float %f1
}

define float @d2f_load_trunc(double * %pv1) nounwind {
entry:
; KNF: vcvtpd2ps $0, {rn}, (%rdi){1to8}, %v0{%k1}
  %v1 = load double * %pv1
  %f1 = fptrunc double %v1 to float
  ret float %f1
}
