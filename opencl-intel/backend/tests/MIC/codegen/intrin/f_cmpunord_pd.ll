; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.cmpunord.pd(<8 x double>, <8 x double>)

define i8 @f_cmpunord_pd(<8 x double> %arg0, <8 x double> %arg1) {
; KNF: f_cmpunord_pd:
; KNF: vcmppd
entry:
  %ret = call i8 @llvm.x86.mic.cmpunord.pd(<8 x double> %arg0, <8 x double> %arg1)

 ret i8 %ret
}

