; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define void @broadcast_load(double* %pSrc, <16 x double>* %pDst) {
entry:
; KNC: vbroadcastsd (%rdi), %zmm0 
; KNC-NOT: vpermd
  %src = load double* %pSrc, align 8
  %a = insertelement <16 x double> undef, double %src, i32 0
  %b = shufflevector <16 x double> %a, <16 x double> undef, <16 x i32> zeroinitializer
  %c = fmul <16 x double> %b, %b
  store <16 x double> %c, <16 x double>* %pDst, align 128 
  ret void
}


