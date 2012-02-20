; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.scatter.step.ps(i8 *, i16, <16 x i32>, <16 x float>, i32, i32, i32)

define i16 @f_scatter_step_ps(i8 * %arg0, i16 %arg1, <16 x i32> %arg2, <16 x float> %arg3, i32 %arg4, i32 %arg5, i32 %arg6) {
; KNF: f_scatter_step_ps:
; KNF: vscatterps
entry:
  %ret = call i16 @llvm.x86.mic.scatter.step.ps(i8 * %arg0, i16 %arg1, <16 x i32> %arg2, <16 x float> %arg3, i32 %arg4, i32 %arg5, i32 %arg6)

 ret i16 %ret
}

