; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i32> @llvm.x86.mic.upconv.int32(i8 *, i32, i32)

define <16 x i32> @f_upconv_int32(i8 * %arg0, i32 %arg1, i32 %arg2) {
; KNF: f_upconv_int32:
; KNF: upconv
entry:
  %ret = call <16 x i32> @llvm.x86.mic.upconv.int32(i8 * %arg0, i32 %arg1, i32 %arg2)

 ret <16 x i32> %ret
}

