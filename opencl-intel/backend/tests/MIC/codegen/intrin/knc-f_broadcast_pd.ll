; XFAIL: win32
; XFAIL: *

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.broadcast.pd(double)

define <8 x double> @f_broadcast_pd(double %arg0) {
; KNF: f_broadcast_pd:
; KNF: vbroadcastpd
entry:
  %ret = call <8 x double> @llvm.x86.mic.broadcast.pd(double %arg0)

 ret <8 x double> %ret
}

