; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.maddn231.pd(<8 x double>, <8 x double>, <8 x double>)

define <8 x double> @f_maddn231_pd(<8 x double> %arg0, <8 x double> %arg1, <8 x double> %arg2) {
; KNF: f_maddn231_pd:
; KNF: vmaddn231pd
entry:
  %ret = call <8 x double> @llvm.x86.mic.maddn231.pd(<8 x double> %arg0, <8 x double> %arg1, <8 x double> %arg2)

 ret <8 x double> %ret
}

