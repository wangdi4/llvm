; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define void @fma_broadcast(double* %pSrc, double* %pDst1, <8 x double>* %pDst2) {
entry:
; KNC: vbroadcastsd (%rdi), %zmm0 
; KNC: vfmadd231pd
; KNC-NOT: vpermd
  %src = load double* %pSrc, align 8
  store double %src, double* %pDst1, align 8 
  %a = insertelement <8 x double> undef, double %src, i32 0
  %b = shufflevector <8 x double> %a, <8 x double> undef, <8 x i32> zeroinitializer
  %c = load <8 x double>* %pDst2, align 64
  %d = fmul <8 x double> %c, %b
  %e = fadd <8 x double> %d, %b
  store <8 x double> %e, <8 x double>* %pDst2, align 64 
  ret void
}

