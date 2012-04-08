; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.sbbr.pi(<16 x i32>, i16, <16 x i32>, i8 *)

define <16 x i32> @f_sbbr_pi(<16 x i32> %arg0, i16 %arg1, <16 x i32> %arg2, i8 * %arg3) {
; KNF: f_sbbr_pi:
; KNF: vsbbrpi
entry:
  %ret = call <16 x i32> @llvm.x86.mic.sbbr.pi(<16 x i32> %arg0, i16 %arg1, <16 x i32> %arg2, i8 * %arg3)

 ret <16 x i32> %ret
}

