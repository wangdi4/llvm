; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.loadunpackh.pd(<8 x double>, i8 *, i32, i32)

define <8 x double> @f_loadunpackh_pd(<8 x double> %arg0, i8 * %arg1) {
; KNF: f_loadunpackh_pd:
; KNF: vloadunpackhq (%{{[a-z]*}}), %{{[0-9]*}}v
entry:
  %ret = call <8 x double> @llvm.x86.mic.loadunpackh.pd(<8 x double> %arg0, i8 * %arg1, i32 0, i32 0)

 ret <8 x double> %ret
}

