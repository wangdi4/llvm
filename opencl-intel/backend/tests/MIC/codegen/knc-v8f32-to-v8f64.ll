; zrackove: I opened the following bug in llvm.org Bugs:
; http://llvm.org/bugs/show_bug.cgi?id=11334

; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define <8 x double> @f2d_ext_vec(<8 x float> %v1) nounwind {
entry:
; KNC: vcvtps2pd %zmm0, %zmm0      
  %f1 = fpext <8 x float> %v1 to <8 x double>
  ret <8 x double> %f1
}
