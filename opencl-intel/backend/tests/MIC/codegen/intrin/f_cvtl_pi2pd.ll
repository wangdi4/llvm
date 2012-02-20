; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <8 x double> @llvm.x86.mic.cvtl.pi2pd(<16 x i32>)

define <8 x double> @f_cvtl_pi2pd(<16 x i32> %arg0) {
; KNF: f_cvtl_pi2pd:
; KNF: cvtl
entry:
  %ret = call <8 x double> @llvm.x86.mic.cvtl.pi2pd(<16 x i32> %arg0)

 ret <8 x double> %ret
}

