; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define void @f2d_ext_store(double * %pv1, float %v0) nounwind {
entry:
; KNF: vcvtps2pd  $0, %v0, [[R1:%v[0-9]+]]{%k1}
; KNF: vstoreq [[R1]]{a}, (%rdi)
  %d1 = fpext float %v0 to double
  store double %d1, double * %pv1, align 8
  ret void
}

define double @f2d_ext(float %v1) nounwind {
entry:
; KNF: vcvtps2pd       $0, %v0, %v0
  %f1 = fpext float %v1 to double
  ret double %f1
}

define double @f2d_load_ext(float * %pv1) nounwind {
entry:
; KNF: vcvtps2pd       $0, (%rdi){1to16}, %v0
  %v1 = load float * %pv1
  %f1 = fpext float %v1 to double
  ret double %f1
}
