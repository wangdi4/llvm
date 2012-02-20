; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.mask.scatter.pd(i8 *, i8, <8 x i64>, <8 x double>, i32, i32, i32)

define void @f_mask_scatter_pd(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, <8 x double> %arg3, i32 %arg4, i32 %arg5, i32 %arg6) {
; KNF: f_mask_scatter_pd:
; KNF: vscatterpd
entry:
  call void @llvm.x86.mic.mask.scatter.pd(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, <8 x double> %arg3, i32 %arg4, i32 %arg5, i32 %arg6)

 ret void 
}

