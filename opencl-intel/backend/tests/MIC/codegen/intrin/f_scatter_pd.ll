; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.scatter.pd(i8 *, <8 x i64>, <8 x double>, i32, i32, i32)

define void @f_scatter_pd(i8 * %arg0, <8 x i64> %arg1, <8 x double> %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_scatter_pd:
; KNF: vscatterpd
entry:
  call void @llvm.x86.mic.scatter.pd(i8 * %arg0, <8 x i64> %arg1, <8 x double> %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret void 
}

