; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.cmp.pd(<8 x double>, <8 x double>, i32)

define i8 @f_cmp_pd(<8 x double> %arg0, <8 x double> %arg1, i32 %arg2) {
; KNF: f_cmp_pd:
; KNF: vcmppd
entry:
  %ret = call i8 @llvm.x86.mic.cmp.pd(<8 x double> %arg0, <8 x double> %arg1, i32 %arg2)

 ret i8 %ret
}

