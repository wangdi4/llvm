; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.mask.cmpord.pd(i8, <8 x double>, <8 x double>)

define i8 @f_mask_cmpord_pd(i8 %arg0, <8 x double> %arg1, <8 x double> %arg2) {
; KNF: f_mask_cmpord_pd:
; KNF: vcmppd
entry:
  %ret = call i8 @llvm.x86.mic.mask.cmpord.pd(i8 %arg0, <8 x double> %arg1, <8 x double> %arg2)

 ret i8 %ret
}

