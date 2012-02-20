; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.cmplt.ps(<16 x float>, <16 x float>)

define i16 @f_cmplt_ps(<16 x float> %arg0, <16 x float> %arg1) {
; KNF: f_cmplt_ps:
; KNF: vcmpps
entry:
  %ret = call i16 @llvm.x86.mic.cmplt.ps(<16 x float> %arg0, <16 x float> %arg1)

 ret i16 %ret
}

