; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare i16 @llvm.x86.mic.gatherpf.step.ps(<16 x i32>, i16, i8 *, i32, i32, i32)

define i16 @f_gatherpf_step_ps(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4, i32 %arg5) {
; KNF: f_gatherpf_step_ps:
; KNF: vgatherpfps
entry:
  %ret = call i16 @llvm.x86.mic.gatherpf.step.ps(<16 x i32> %arg0, i16 %arg1, i8 * %arg2, i32 %arg3, i32 %arg4, i32 %arg5)

 ret i16 %ret
}

