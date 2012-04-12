; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.addsets.pi(<16 x i32>, <16 x i32>, i8 *)

define <16 x i32> @f_addsets_pi(<16 x i32> %arg0, <16 x i32> %arg1, i8 * %arg2) {
; KNF: f_addsets_pi:
; KNF: vaddsetspi %v{{[0-9]*}}, %v{{[0-9]*}}, %v{{[0-9]*}}{%k{{[0-9]*}}}
entry:
  %ret = call <16 x i32> @llvm.x86.mic.addsets.pi(<16 x i32> %arg0, <16 x i32> %arg1, i8 * %arg2)

 ret <16 x i32> %ret
}

