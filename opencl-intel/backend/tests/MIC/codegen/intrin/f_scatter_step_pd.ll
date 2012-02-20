; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i8 @llvm.x86.mic.scatter.step.pd(i8 *, i8, <8 x i64>, <8 x double>, i32, i32, i32)

define i8 @f_scatter_step_pd(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, <8 x double> %arg3, i32 %arg4, i32 %arg5, i32 %arg6) {
; KNF: f_scatter_step_pd:
; KNF: vscatterpd
entry:
  %ret = call i8 @llvm.x86.mic.scatter.step.pd(i8 * %arg0, i8 %arg1, <8 x i64> %arg2, <8 x double> %arg3, i32 %arg4, i32 %arg5, i32 %arg6)

 ret i8 %ret
}

