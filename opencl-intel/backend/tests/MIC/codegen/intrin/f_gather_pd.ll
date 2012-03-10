; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.gather.pd(<8 x i64>, i8 *, i32, i32, i32)

define <8 x double> @f_gather_pd(<8 x i64> %arg0, i8 * %arg1) {
; KNF: f_gather_pd:
; KNF: vshuf128x32 $80, $80, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vshuf128x32 $250, $80, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vgatherd (%{{[a-z]*}},%v{{[0-9]*}},4), %v{{[0-9]*}}{%k{{[0-9]*}}}
; KNF: vkortest %k{{[0-9]*}}, %k{{[0-9]*}}
; KNF: vorpi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <8 x double> @llvm.x86.mic.gather.pd(<8 x i64> %arg0, i8 * %arg1, i32 0, i32 8, i32 0)

 ret <8 x double> %ret
}

